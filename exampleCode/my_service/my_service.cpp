#include <iostream>

#include "HBBconfig.h" // 一些配置的宏，例如路由中心的端口，

#include "HBBservice.h" // 提给给服务端的接口
#include "HBBlog_client.h" // 导入日志中心

#include "mymsg.pb.h" // 自己定义的消息信息


// 微服务名词，用于进行路由转发
#define SERVER1 "SERVER_1" // 第一种服务模块的名称
using namespace std;
class myHandle :public HBBServiceHandle
{
	// 一个HBBLogHanlde1 负责与一个 client 通信
public:
	myHandle()
	{
		RegMsgCallback();
	}
	// 处理消息 MSG_REQ1 的回调函数
	void processMsg1(HBBmsg::HBBMsgHead* head, Msg* msg) {
		myReqMsg req;
		if (!req.ParseFromArray(msg->data,msg->size)) {
			LOGERROR("myReqMsg::ParseFromArray failed!");// 提供的日志接口
		}
		// 打印日志接口的信息
		cout <<"processMsg1 收的消息 : "<< req.DebugString() << endl;
		// 返回消息
		myResMsg res;
		res.set_service_name(SERVER1);
		res.set_text("hello ,message from myHandle1::processMsg1");
		// 发送消息回去
		SendMsg((HBBmsg::MsgType)(myMsgType::MSG_RES1),&res);
	}
	void processMsg2(HBBmsg::HBBMsgHead* head, Msg* msg) {
		myReqMsg req;
		if (!req.ParseFromArray(msg->data, msg->size)) {
			LOGERROR("myReqMsg::ParseFromArray failed!");// 提供的日志接口
		}
		// 打印日志接口的信息
		cout << "processMsg2 收的消息 : " << req.DebugString() << endl;
		// 返回消息
		myResMsg res;
		res.set_service_name(SERVER1);
		res.set_text("hello ,message from myHandle1::processMsg2");
		// 发送消息回去
		SendMsg((HBBmsg::MsgType)(myMsgType::MSG_RES1), &res);
	}
	static void RegMsgCallback() {
		RegCB(
			(HBBmsg::MsgType)myMsgType::MSG_REQ1
			, (MsgCBFunc)&myHandle::processMsg1
		);
		RegCB(
			(HBBmsg::MsgType)myMsgType::MSG_REQ2
			, (MsgCBFunc)&myHandle::processMsg2
		);
	}

};
class MyServer :public HBBService
{
public:
	// 采用工厂模式，每次有新的连接建立，返回一个 HBBLogHandle1 对象与新的连接通信
	HBBServiceHandle *CreateServiceHandle() override
	{
		return new myHandle();
	}
};
int main()
{
	std::cout << "HBBLOG SERVER" << std::endl;
	// 初始化日志服务
	int server_port = 18888; // 这个服务在 1888号端口上运行
	InitLog("127.0.0.1", LOG_PORT, server_port, SERVER1); // 连接日志中心，不启动的话本地控制台打印
	// 向注册中心注册
	myHandle::RegMsgCallback(); //  注册消息回调
	MyServer server;
	server.set_arg(server_port,SERVER1,"127.0.0.1",REGISTER_PORT); // 向注册中心注册,第三个参数为注册中心地址，假设在本地
	server.Start(); // 开始监听
	while (1);
	return 0;
}

