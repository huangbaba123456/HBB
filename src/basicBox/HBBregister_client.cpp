
#include "HBBregister_client.h"
#include "HBBmsg.pb.h"
#include "HBBtools.h"
#include <thread>
#include <fstream>
#include <sstream>
#include "HBBlog_client.h"

using namespace HBBmsg;

using namespace std;

///注册服务列表的缓存
static HBBServiceMap * service_map = 0;
static HBBServiceMap * client_map = 0;

//多线程访问的锁
static mutex service_map_mutex;

/////////////////////////////////////////////////////////////
///发出有获取微服务列表的请求
///@para service_name == NULL 则取全部
void HBBRegisterClient::GetServiceReq(const char *service_name)
{

    HBBGetServiceReq req;
    if (service_name)
    {
        req.set_type(HBBServiceType::ONE);
        req.set_name(service_name);
    }
    else
    {
        req.set_type(HBBServiceType::ALL);
    }

    SendMsg(MSG_GET_SERVICE_REQ, &req);
}
/////////////////////////////////////////////////////////////////////////////
/// 获取指定服务名称的微服务列表 （阻塞函数）
/// 1 等待连接成功 2 发送获取微服务的消息 3 等待微服务列表消息反馈（有可能拿到上一次的配置）
/// @para service_name 服务名称
/// @para timeout_sec 超时时间
/// @return 服务列表
HBBmsg::HBBServiceMap::HBBServiceList HBBRegisterClient::GetServcies(const char *service_name, int timeout_sec)
{
    HBBmsg::HBBServiceMap::HBBServiceList re;
    //十毫米判断一次
    int totoal_count = timeout_sec * 100;
    int count = 0;
    //1 等待连接成功
    while (count < totoal_count)
    {
        //cout << "@" << flush;
        if (is_connected())
            break;
        this_thread::sleep_for(chrono::milliseconds(10));
        count++;
    }
	// 需要判断是否service_name是否为空
    if (!is_connected())
    {
        LOGDEBUG("连接等待超时");
		{
			HBBMutex mutex(&service_map_mutex);
			if (!service_map) {
				service_map = new HBBServiceMap();
			}
			LoadLocalFile();
			if (service_map) {
				auto m = service_map->mutable_service_map();
				auto s = m->find(service_name);
				if (s != m->end()) {
					re.CopyFrom(s->second);
				}
			}
		}
        return re;
    }

    //2 发送获取微服务的消息
    GetServiceReq(service_name);
    
    //3 等待微服务列表消息反馈（有可能拿到上一次的配置）
    while (count < totoal_count)
    {
        cout << "." << flush;
        HBBMutex mutex(&service_map_mutex);
        if (!service_map)
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            count++;
            continue;
        }
        auto m = service_map->mutable_service_map();
        if (!m)
        {
            //cout << "#" << flush;
            //没有找到指定的微服务
            GetServiceReq(service_name);
            this_thread::sleep_for(chrono::milliseconds(100));
            count+=10;
            continue;
        }
        auto s = m->find(service_name);
        if (s == m->end())
        {
           // cout << "+" << flush;
            //没有找到指定的微服务
            GetServiceReq(service_name);
            this_thread::sleep_for(chrono::milliseconds(100));
            count += 10;
            continue;
        }
        re.CopyFrom(s->second);
        return re;
    }
    return re;

    //XMutex mutex(&service_map_mutex);
}

HBBmsg::HBBServiceMap *HBBRegisterClient::GetAllService()
{
    HBBMutex mutex(&service_map_mutex);
	if (!service_map) {
		LoadLocalFile();
	}
    if (!service_map)
    {
        return NULL;
    }
    if (!client_map)
    {
        client_map = new HBBServiceMap();
    }
    client_map->CopyFrom(*service_map);
    return client_map;
}
//获取服务列表的响应
void HBBRegisterClient::GetServiceRes(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    LOGDEBUG("GetServiceRes");

	HBBServiceMap tmp_map;
    if (!tmp_map.ParseFromArray(msg->data, msg->size)){
        LOGDEBUG("service_map.ParseFromArray failed!");
        return;
    }
	if (tmp_map.mutable_res()->return_()==HBBMessageRes::HBBReturn::HBBMessageRes_HBBReturn_ERROR) {
		//消息传输错误啦
		LOGDEBUG("HBBMessageRes::HBBReturn::HBBMessageRes_HBBReturn_ERROR "+tmp_map.mutable_res()->msg());
		return;
	}
    //LOGDEBUG(service_map->DebugString());
	// 要区分是哪个
	{
		HBBMutex mutex(&service_map_mutex);
		if (!service_map) {
			service_map = new HBBServiceMap();
		}
	}
	
	if (tmp_map.type()== HBBServiceType::ONE) {
		// 一个的
		if (tmp_map.mutable_service_map()->size() != 0) {
			auto m = tmp_map.mutable_service_map()->begin(); // 取第一个
			{
				HBBMutex mutex(&service_map_mutex);
				(*(service_map->mutable_service_map()))[m->first] = m->second; // 加入进去
			}
		}
		
	}else {
		// all的
		{
			HBBMutex mutex(&service_map_mutex);
			service_map->CopyFrom(tmp_map);
		}
	}
	// 下面是磁盘缓存
	string fileName = getLocalFileName();
	LOGDEBUG("Save local file!");
	ofstream ofs;
	ofs.open(fileName,ios::binary);
	if (!ofs.is_open()) {
		// 打开失败
		LOGDEBUG("save local file failed!");
	}else {
		service_map->SerializePartialToOstream(&ofs);
	}
	ofs.close();

}
//接收服务的注册响应
void HBBRegisterClient::RegisterRes(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    LOGDEBUG("RegisterRes");
    HBBMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("HBBRegisterClient::RegisterRes failed!res.ParseFromArray failed!");
        return;
    }
    if (res.return_() == HBBMessageRes::OK)
    {
        LOGDEBUG("RegisterRes success");
        return;
    }
    stringstream ss;
    ss << "RegisterRes failed! " << res.msg();
    LOGDEBUG(ss.str().c_str());
}

void HBBRegisterClient::ConnectedCB()
{
    //发送注册消息
    LOGDEBUG("connected start send MSG_REGISTER_REQ ");
    HBBRegisterReq req;
    req.set_name(service_name_);
    req.set_ip(service_ip_);
    req.set_port(service_port_);
    SendMsg(MSG_REGISTER_REQ, &req);
}

///////////////////////////////////////////////////////////////////////
//// 向注册中心注册服务 
/// @para service_name 微服务名称
/// @para port 微服务接口
/// @para ip 微服务的ip，如果传递NULL，则采用客户端连接地址
void HBBRegisterClient::RegisterServer(const char *service_name, int port, const char *ip)
{
    //注册消息回调函数
    RegMsgCallback();
    //发送消息到服务器
    //服务器连接是否成功？
    ///注册中心的IP，注册中心的端口
    //_CRT_SECURE_NO_WARNINGS
    if (service_name)
        strcpy(service_name_, service_name);
    if (ip)
        strcpy(service_ip_, ip);
    service_port_ = port;
    //设置自动重连
    set_auto_connect(true);
	//  设置定时任务，这个任务其实就是发送心跳包
	set_timer_ms(3000);
	// 设置读超时
	// set_read_timeout_ms(5000);
    // 把任务加入到线程池中
    StartConnect();

}

HBBRegisterClient::HBBRegisterClient()
{
}


HBBRegisterClient::~HBBRegisterClient()
{
}

bool HBBRegisterClient::LoadLocalFile() {
	bool ans;
	LOGDEBUG("Load local register data");
	if (!service_map) {
		service_map = new HBBServiceMap();
	}
	string name = getLocalFileName();
	ifstream ifs;
	ifs.open(name, ios::binary);
	if (ifs.is_open()) {
		ans = service_map->ParseFromIstream(&ifs);
		LOGDEBUG("Load local register data success");
	}else {
		ans = false;
	}
	if (!ans) {
		stringstream log;
		log << "Load local register data failed! fileName: ";
		log << name;
		LOGDEBUG(log.str());
		delete service_map;
		service_map = 0;
	}
	ifs.close();
	return ans;
}
string HBBRegisterClient::getLocalFileName() {
	stringstream ss;
	ss << "register_"<<service_name_<< "_" << service_ip_ << "_" << service_port_ << ".cache";
	return ss.str();
}

//定时器，用于发送心跳
void HBBRegisterClient::TimerCB()
{
	static long long count = 0;
	count++;
	HBBMsgHeart req;
	req.set_count(count);
	SendMsg(MSG_HEART_REQ, &req);
}

