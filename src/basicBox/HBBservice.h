
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

    //需要重载，每个连接进入，调用此函数创建处理对象，加入到线程池
    virtual HBBServiceHandle* CreateServiceHandle() = 0;
    ///服务初始化 由线程池调用
    bool Init();

    ///开始服务运行，接收连接任务加入到线程池
    bool Start();

    //服务器监听端口
    void set_server_port(int port) { this->server_port_ = port; }
	virtual void set_arg(int server_port, std::string register_name, std::string register_ip, int register_port) {
		HBBRegisterClient::Get()->set_server_ip(register_ip.c_str());
		HBBRegisterClient::Get()->set_server_port(register_port);
		HBBRegisterClient::Get()->RegisterServer(register_name.c_str(), server_port, 0);
		// 设置服务端口
		set_server_port(server_port);
	};
    void ListenCB(int client_socket, struct sockaddr *addr, int socklen);
private:
    
    //接收用户连接的线程池
    HBBThreadPool *thread_listen_pool_ = 0;

    //处理用户的数据的连接池
    HBBThreadPool *thread_client_pool_ = 0;

    //客户数据处理的线程数量
    int thread_count_ = 10;

    //服务器监听端口
    int server_port_ = 0;


};

#endif