#ifndef HBBSERVICEPROXY_H
#define HBBSERVICEPROXY_H
#include <map>
#include <vector>
#include <string>
#include "HBBservice_proxy_client.h"
#include "HBBrouter_handle.h"
class HBBServiceProxy
{
public:
    static HBBServiceProxy *Get()
    {
        static HBBServiceProxy xs;
        return &xs;
    }
    HBBServiceProxy();
    ~HBBServiceProxy();
    ///初始化微服务列表（注册中心获取），建立连接
    bool Init();

    //负载均衡找到客户端连接，进行数据发送
    bool SendMsg(HBBmsg::HBBMsgHead *head, Msg *msg, HBBRouterHandle *ev);



    //开启自动重连的线程
    void Start();

    //停止线程
    void Stop();

    void Main();

private:

    bool is_exit_ = false;
	std::map < std::string, std::vector<HBBmsg::HBBServiceMap::HBBService>> service_map_;
	std::mutex service_map_mutex_;
	//记录上次轮询的索引
	std::map<std::string, int>service_map_last_index_;

};

#endif