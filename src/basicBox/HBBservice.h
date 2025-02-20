
#ifndef HBBSERVICE_H_
#define HBBSERVICE_H_
#include "HBBtask.h"
#include "HBBservice_handle.h"
#include "HBBthread_pool.h"
#include "HBBregister_client.h"
class  HBBService :public HBBTask
{
public:
    HBBService();
    ~HBBService();

    //��Ҫ���أ�ÿ�����ӽ��룬���ô˺�������������󣬼��뵽�̳߳�
    virtual HBBServiceHandle* CreateServiceHandle() = 0;
    ///�����ʼ�� ���̳߳ص���
    bool Init();

    ///��ʼ�������У���������������뵽�̳߳�
    bool Start();

    //�����������˿�
    void set_server_port(int port) { this->server_port_ = port; }
	virtual void set_arg(int server_port, std::string register_name, std::string register_ip, int register_port) {
		HBBRegisterClient::Get()->set_server_ip(register_ip.c_str());
		HBBRegisterClient::Get()->set_server_port(register_port);
		HBBRegisterClient::Get()->RegisterServer(register_name.c_str(), server_port, 0);
		// ���÷���˿�
		set_server_port(server_port);
	};
    void ListenCB(int client_socket, struct sockaddr *addr, int socklen);
private:
    
    //�����û����ӵ��̳߳�
    HBBThreadPool *thread_listen_pool_ = 0;

    //�����û������ݵ����ӳ�
    HBBThreadPool *thread_client_pool_ = 0;

    //�ͻ����ݴ�����߳�����
    int thread_count_ = 10;

    //�����������˿�
    int server_port_ = 0;


};

#endif