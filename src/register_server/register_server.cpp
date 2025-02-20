#include <iostream>
#include "HBBregister_handle.h"
#include "HBBregister_server.h"
#include "HBBconfig.h"
using namespace std;
int main(int argc, char *argv[])
{
    cout << "Register Server" << endl;
    HBBRegisterServer server;
	HBBRegisterHandle::RegMsgCallback();
	//InitLog("127.0.0.1", LOG_PORT,REGISTER_PORT,REGISTER_SERVER_NAME);
    //初始化 传递参数，端口号 register_server 20018
    server.main(REGISTER_PORT);
    //启动服务线程，开始监听端口
    server.Start();
    //阻塞，等待线程池退出
    server.Wait();
    return 0;
}