#include "HBBservice_proxy.h"
#include "HBBmsg.pb.h"
#include "HBBtools.h"
#include "HBBregister_client.h"
#include <thread>
#include "HBBlog_client.h"
using namespace std;
using namespace HBBmsg;
///初始化微服务列表（注册中心获取），建立连接
bool HBBServiceProxy::Init()
{

    return true;
}

//负载均衡找到客户端连接，进行数据发送
bool HBBServiceProxy::SendMsg(HBBmsg::HBBMsgHead *head, Msg *msg, HBBRouterHandle *ev)
{
    if (!head || !msg)return false;
	if (ev->serverclient()) {
		if (ev->serverclient()->is_connected()) {
			// 连接好啦
			LOGDEBUG("直接发送");
			ev->serverclient()->HBBMsgEvent::SendMsg(head,msg); // 直接发送
			return true;
		}else if (ev->serverclient()->is_connecting()) {
			// 正在连接中，挂在消息队列里面
			LOGDEBUG("挂载在消息队列里面");
			ev->serverclient()->addMsg(head,msg);
			return true;
		}
		else {
			// 已经断开连接啦
			//ev->set_serverclient(NULL);
			LOGDEBUG("挂载连接已经断开");
			ev->Close();
			return false;
		}
	
	}

	string service_name = head->service_name();
	string service_ip =""; // ip地址
	int service_port = -1; // 端口号
	{
		HBBMutex mtx(&service_map_mutex_);
		auto service_list = service_map_.find(service_name);
		if (service_list == service_map_.end()) {
			LOGDEBUG("service: " + service_name + " not find");
			return false;
		}
		int cur_index = service_map_last_index_[service_name];
		// 取出对应微服务
		HBBServiceMap::HBBService service = service_list->second[cur_index]; // 取出这个微服务
		// 得到对应的ip 和 port
		service_ip = service.ip(); // ip地址
		service_port = service.port(); // 端口号
		service_map_last_index_[service_name] = ((cur_index + 1) % service_list->second.size()); // 跟新连接
	}
	auto proxy=HBBServiceProxyClient::Create(service_name);
	proxy->set_server_ip(service_ip.c_str());
	proxy->set_server_port(service_port);
	proxy->set_auto_delete(false); // 不要自动删除，等XRouterHandle 删除它
	proxy->set_xrouterhandle(ev);
	ev->set_serverclient(proxy);
	proxy->StartConnect();

	// 判断是否连接
	if (ev->serverclient()) {
		if (ev->serverclient()->is_connected()) {
			// 连接好啦
			ev->serverclient()->HBBMsgEvent::SendMsg(head, msg); // 直接发送
			LOGDEBUG("直接发送");
			return true;
		}
		else if (ev->serverclient()->is_connecting()) {
			// 正在连接中，挂在消息队列里面
			LOGDEBUG("挂载在消息队列里面");
			ev->serverclient()->addMsg(head, msg);
			return true;
		}
		else {
			// 已经断开连接啦
			//ev->set_serverclient(NULL);
			ev->Close();
			LOGDEBUG("挂载连接已经断开");
			return false;
		}
	}
    return true;
}


//开启自动重连的线程
void HBBServiceProxy::Start()
{
    thread th(&HBBServiceProxy::Main, this);
    th.detach();
}

//停止线程
void HBBServiceProxy::Stop()
{

}

void HBBServiceProxy::Main()
{
    //自动重连
    while (!is_exit_)
    {
        // 从注册中心获取 微服务的列表更新
        //发送请求到注册中心
        HBBRegisterClient::Get()->GetServiceReq(0);
        auto service_map = HBBRegisterClient::Get()->GetAllService();

        if (!service_map)
        {
            LOGDEBUG("GetAllService : service_map is NULL");
            this_thread::sleep_for(1s);
            continue;
        }
        auto smap = service_map->service_map();

        if (smap.empty())
        {
            LOGDEBUG("HBBServiceProxy : service_map->service_map is NULL");
            this_thread::sleep_for(1s);
            continue;
        }
		cout << service_map->DebugString() << endl;
		{
			HBBMutex mux(&service_map_mutex_);
			service_map_.clear();
			for (auto m : smap) {
				service_map_[m.first] = vector<HBBServiceMap::HBBService>();
				//
				for (auto s : m.second.service()) {
					service_map_[m.first].push_back(s);
				}
				service_map_last_index_[m.first] = 0;
			}
			
		}
		this_thread::sleep_for(3000ms);
    }
}
HBBServiceProxy::HBBServiceProxy()
{

}


HBBServiceProxy::~HBBServiceProxy()
{

}
