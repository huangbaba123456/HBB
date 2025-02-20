#include <iostream>
#include "HBBrouter_server.h"
#include "HBBservice_proxy.h"
#include "HBBconfig.h"
#include "HBBlog_client.h"
using namespace std;
int main(int argc, char *argv[])
{
	/// xms_gateway
	cout << "gateway" << endl;
	// 初始化日志
	HBBServiceProxy::Get()->Init();
	//开启自动重连
	HBBServiceProxy::Get()->Start();
	HBBRouterServer service;
	//InitLog("127.0.0.1", LOG_PORT, API_GATEWAY_PORT, API_GATEWAY_SERVER_NAME);
	service.set_arg(API_GATEWAY_PORT,API_GATEWAY_SERVER_NAME,HBBGetHostByName(REGISTER_SERVER_NAME),REGISTER_PORT); // 设置与他的连接
	////service.set_arg(API_GATEWAY_PORT, API_GATEWAY_NAME, "127.0.0.1", REGISTER_PORT); // 设置与他的连接
	service.Start();
	HBBThreadPool::Wait();
	return 0;
}