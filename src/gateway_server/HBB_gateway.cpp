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
	// ��ʼ����־
	HBBServiceProxy::Get()->Init();
	//�����Զ�����
	HBBServiceProxy::Get()->Start();
	HBBRouterServer service;
	//InitLog("127.0.0.1", LOG_PORT, API_GATEWAY_PORT, API_GATEWAY_SERVER_NAME);
	service.set_arg(API_GATEWAY_PORT,API_GATEWAY_SERVER_NAME,HBBGetHostByName(REGISTER_SERVER_NAME),REGISTER_PORT); // ��������������
	////service.set_arg(API_GATEWAY_PORT, API_GATEWAY_NAME, "127.0.0.1", REGISTER_PORT); // ��������������
	service.Start();
	HBBThreadPool::Wait();
	return 0;
}