

#include <iostream>
#include <thread>
#include "HBBlog_client.h" // 导入日志接口
#include "HBBconfig.h" // 一些配置的宏，例如路由中心的端口，
#include "HBBservice_client.h" // 我提供给客户端的接口


#include "mymsg.pb.h" // 自己定义的消息信息


// 微服务名词，用于进行路由转发
#define SERVER1 "SERVER_1" // 服务模块的名称

using namespace std;
//using namespace HBB;
class MyClient :public HBBServiceClient {
public:
	// 单例模式构造对象
	static MyClient* Get() {
		static MyClient client;
		return &client;
	}
	// 删除复制构造函数
	MyClient(const MyClient&) = delete;
	MyClient(MyClient&& ) = delete;
	// 连接建立的回调函数
	virtual void ConnectedCB() override {
		
	}

	// 设置消息回调函数
	static void RegMsgCallback() {
		// 注册消息回调函数
		RegCB((HBBmsg::MsgType)myMsgType::MSG_RES1,(MsgCBFunc)&MyClient::MessageResponse1);
		RegCB((HBBmsg::MsgType)myMsgType::MSG_RES2, (MsgCBFunc)&MyClient::MessageResponse2);
	}
private:
	MyClient() = default;
	
	// 收到消息 myMsgType::MSG_RES1 的回调函数 
	void MessageResponse1(HBBmsg::HBBMsgHead* head, Msg* msg) {
		// 解析消息体
		myResMsg resMsg;
		if (!resMsg.ParseFromArray(msg->data,msg->size)) {
			LOGERROR("myResMsg::ParseFromArray failed!");// 提供的日志接口
			return;
		}
		string res_service_name=resMsg.service_name();
		string res_text=resMsg.text();
		cout <<"MessageResponse_1 收到来自于微服务: " << res_service_name << "  的消息：" << res_text<<endl;
		
	}

	// 收到消息 myMsgType::MSG_RES2 的回调函数 
	void MessageResponse2(HBBmsg::HBBMsgHead* head, Msg* msg) {
		// 解析消息体
		myResMsg resMsg;
		if (!resMsg.ParseFromArray(msg->data, msg->size)) {
			LOGERROR("myResMsg::ParseFromArray failed!");// 提供的日志接口
			return;
		}
		string res_service_name = resMsg.service_name();
		string res_text = resMsg.text();
		cout << "MessageResponse_2 收到来自于微服务: " << res_service_name << "  的消息：" << res_text<<endl;
	}
};

int main()
{
	cout << "************** MyClient ****************" << endl;
	#define CON MyClient::Get()
	MyClient::RegMsgCallback(); // 注册消息回调
	InitLog("127.0.0.1", LOG_PORT, -1, "my_client"); // 连接日志中心，不启动的话本地控制台打印
	CON->set_server_ip("127.0.0.1"); // 路由中心在本地
	CON->set_server_port(API_GATEWAY_PORT); // 路由中心的端口
	CON->set_auto_connect(true); // 设置自动重连
	CON->StartConnect(); // 开始连接
	this_thread::sleep_for(3s); 
	while (!CON->is_connected()) { // 等待连接成功
		cout << "等待连接" << endl;
		this_thread::sleep_for(10ms); 
	}
	cout << "连接路由中心成功" << endl;
	{
		// 发送消息给 SERVER1  模块
		while (1)
		{
			CON->set_service_name(SERVER1); // 消息转发给 SERVER1 模块
			myReqMsg reqMsg; // 自己定义的请求消息
			reqMsg.set_client_name("myclient");
			reqMsg.set_text("hello world");
			CON->SendMsg((HBBmsg::MsgType)myMsgType::MSG_REQ1, &reqMsg); //发送消息类型为 MSG_REQ1 的消息给 SERVER1
			CON->SendMsg((HBBmsg::MsgType)myMsgType::MSG_REQ2, &reqMsg); //发送消息类型为 MSG_REQ2 的消息给 SERVER1
			this_thread::sleep_for(1s);
		}

	}
	// 等待消息回复
	while (1);
	return 0;
}

