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
    //��ʼ�� ���ݲ������˿ں� register_server 20018
    server.main(REGISTER_PORT);
    //���������̣߳���ʼ�����˿�
    server.Start();
    //�������ȴ��̳߳��˳�
    server.Wait();
    return 0;
}