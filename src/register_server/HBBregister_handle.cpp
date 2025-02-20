
#include "HBBregister_handle.h"
#include "HBBtools.h"
#include "HBBmsg.pb.h"
#include "HBBlog_client.h"

using namespace HBBmsg;
using namespace std;

///注册服务列表的缓存
static HBBServiceMap * service_map = 0;

//多线程访问的锁
static mutex service_map_mutex;


//接收服务的发现请求
void HBBRegisterHandle::GetServiceReq(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    //暂时只发送全部
    LOGDEBUG("接收服务的发现请求");
    HBBGetServiceReq req;

    //错误处理
    HBBmsg::HBBServiceMap res;
    res.mutable_res()->set_return_(HBBMessageRes_HBBReturn_ERROR);
	res.set_type(req.type()); // 强制转换
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        stringstream ss;
        ss << "req.ParseFromArray failed!";
        LOGDEBUG(ss.str().c_str());
        res.mutable_res()->set_msg(ss.str().c_str());
        SendMsg(MSG_GET_SERVICE_RES, &res);
        return;
    }

    string service_name = req.name();
    stringstream ss;
    ss << "GetServiceReq : service name " << service_name;
    LOGDEBUG(ss.str().c_str());
	{
		//发送全部微服务数据
		HBBMutex mtx(&service_map_mutex);
		if (!service_map) {
			service_map = new HBBServiceMap();
		}
		HBBServiceMap sendMap;
		sendMap.mutable_res()->set_return_(HBBMessageRes_HBBReturn_OK);
		sendMap.set_type(req.type());
		if (req.type() == HBBServiceType::ALL) {
			// 发送全部
			sendMap.CopyFrom(*service_map);
			sendMap.set_type(req.type()); // 之前这一行忘记写啦好尴尬
		}else {
			// 发单独的服务
			auto tmpMap=service_map->mutable_service_map();
			if (tmpMap->find(service_name) != tmpMap->end()) {
				//
				(*sendMap.mutable_service_map())[service_name] = (*tmpMap)[service_name];
			}
		}
        LOGDEBUG(sendMap.DebugString());
		SendMsg(MSG_GET_SERVICE_RES, &sendMap);
	}
}

//接收服务的注册请求
void HBBRegisterHandle::RegisterReq(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    LOGDEBUG("服务端接收到用户的注册请求");

    //回应的消息
    HBBMessageRes res;
    
    //解析请求
    HBBRegisterReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("HBBRegisterReq ParseFromArray failed!");
        res.set_return_(HBBMessageRes::ERROR);
        res.set_msg("HBBRegisterReq ParseFromArray failed!");
        SendMsg(MSG_REGISTER_RES, &res);
        return;
    }

    
    //接收到用户的服务名称、服务IP、服务端口
    string service_name = req.name();
    if (service_name.empty())
    {
        string error = "service_name is empty!";
        LOGDEBUG(error.c_str());
        res.set_return_(HBBMessageRes::ERROR);
        res.set_msg(error);
        SendMsg(MSG_REGISTER_RES, &res);
        return;
    }
    string service_ip = req.ip();
    if (service_ip.empty())
    {
        LOGDEBUG("service_ip is empty : client ip");
        service_ip = this->client_ip();
    }

    int service_port = req.port();
    if (service_port <= 0 || service_port > 65535)
    {
        stringstream ss;
        //string error = "service_port is error!";
        ss << "service_port is error!" << service_port;
        LOGDEBUG(ss.str().c_str());
        res.set_return_(HBBMessageRes::ERROR);
        res.set_msg(ss.str());
        SendMsg(MSG_REGISTER_RES, &res);
        return;
    }
    
    //接收用户注册信息正常
    stringstream ss;
    ss << "接收到用户注册信息:" << service_name << "|" << service_ip << ":" << service_port;
    LOGINFO(ss.str().c_str());


    //存储用户注册信息，如果已经注册需要更新
    {
        HBBMutex mutex(&service_map_mutex);
        if (!service_map)
            service_map = new HBBServiceMap();
        //map的指针
        auto smap = service_map->mutable_service_map();

        //是否由同类型已经注册
        //集群微服务
        auto service_list = smap->find(service_name);
        if (service_list == smap->end())
        {
            //没有注册过
            (*smap)[service_name] = HBBServiceMap::HBBServiceList();
            service_list = smap->find(service_name);
        }
        auto services = service_list->second.mutable_service();
        //查找是否用同ip和端口的
        for (auto service : (*services))
        {
            if (service.ip() == service_ip && service.port() == service_port)
            {
                stringstream ss;
                ss <<service_name<<"|"<< service_ip << ":"
                    << service_port << "微服务已经注册过";
                LOGDEBUG(ss.str().c_str());
                res.set_return_(HBBMessageRes::ERROR);
                res.set_msg(ss.str());
                SendMsg(MSG_REGISTER_RES, &res);
                return;
            }
        }
        //添加新的微服务
        auto ser  = service_list->second.add_service();
        ser->set_ip(service_ip);
        ser->set_port(service_port);
        ser->set_name(service_name);
        stringstream ss;
        ss << service_name << "|" << service_ip << ":"
            << service_port << "新的微服务注册成功！";
        LOGDEBUG(ss.str().c_str());
    }


    res.set_return_(HBBMessageRes::OK);
    res.set_msg("OK");
    SendMsg(MSG_REGISTER_RES, &res);
}

HBBRegisterHandle::HBBRegisterHandle()
{
}


HBBRegisterHandle::~HBBRegisterHandle()
{
}
