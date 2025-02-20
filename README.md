# HBB框架使用说明

## 编译

请首先安装***libevent,openssl,protobuf***库，并且安装***mysql***。

编译安装基础库以及**路由中心，注册中心，日志中心**可执行文件

```bash
mkdir build
cd build
cmake ..
make
```

安装成功后，bin目录下出现三个可执行文件***gateway_server ，log_server ，register_server***,lib目录下出现两个静态库


启动**路由中心，注册中心，日志中心**

```bash
cd ../
python run.py
```

启动成功后显示

![本地路径](image/python.png "客户端")

## 通信示例

编译

```bash
cd exampleCode/
mkdir build
cd build
cmake ..
make
cd ..
```

此时exampleCode/bin目录下出现两个可执行文件***my_client ，my_service***

```bash
hbb@ubuntu:~/githubcode/exampleCode$ ls bin
my_client  my_service
```

运行服务端

```bash
./bin/my_service
```



![本地路径](image/serverResult.png "服务器")

运行客户端

```bash
./bin/my_client 
```

![本地路径](image/clientResult.png "客户端")

## 示例代码说明

#### 利用protobuf定义双方将要传送的消息

框架基于protobuf通信，首先需要定义消息类型，客户端和服务器均根据消息类型调用对应的处理回调函数，消息类型存储与消息头中，然后定义消息体，消息体是任意根据protobuf定义生成的c++类。

***`mymsg.proto`***

```protobuf
syntax="proto3";
// 自己定义消息类型，微服务实例应用消息类型提供对应处理
// 自定义枚举值从128开始,1-127供框架内部使用（用户不可使用）
enum myMsgType{
	MSG_NO_USE = 0;  // proto3 必须以 0 开始，作为默认值
	MSG_REQ1=128;
	MSG_RES1=129;
	MSG_REQ2=130;
	MSG_RES2=131;
	MSG_REQ3=132;
	MSG_RES3=133;
};

// 请求消息类型,自己定义
// 客户端发送给服务器
message myReqMsg{
	string client_name=1;
	bytes text=2;
};

// 请求恢复消息类型,自己定义
// 服务器发送给客户端
message myResMsg{
	string service_name=1;
	bytes text=2;
};
```

生成***mymsg.pb.cc,mymsg.pb.h***文件，将这两个文件分别置于客户端和服务端的文件夹下面。

#### 客户端部分

​	需要继承类HBBServiceClient，HBBServiceClient使用观察者模式设计模式，子类调用RegCB函数完成消息回调注册，客户端与路由中心建立连接，并且要调用set_service_name()函数设置所需的微服务名，路由中心根据微服务名转发给对应微服务功能模块。

***`my_client.cpp`***

```c++
#include <iostream>

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
```

#### 服务端部分

服务端在启动监听端口前，需要向注册中心注册直接的微服务信息。

服务端部分采用工厂设计模式和客户端通信，需要继承HBBService，实现它的虚函数CreateServiceHandle，

每当有新的连接建立的时候CreateServiceHandle会返回一个继承抽象类HBBServiceHandle的子类，子类中处理与客户端通信。

***`my_service.cpp`***

```c++
#include <iostream>

#include "HBBconfig.h" // 一些配置的宏，例如路由中心的端口，

#include "HBBservice.h" // 提给给服务端的接口
#include "HBBlog_client.h" // 导入日志中心

#include "mymsg.pb.h" // 自己定义的消息信息


// 微服务名词，用于进行路由转发
#define SERVER1 "SERVER_1" // 第一种服务模块的名称
using namespace std;
class myHandle1 :public HBBServiceHandle
{
	// 一个HBBLogHanlde1 负责与一个 client 通信
public:
	myHandle1()
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
			, (MsgCBFunc)&myHandle1::processMsg1
		);
		RegCB(
			(HBBmsg::MsgType)myMsgType::MSG_REQ2
			, (MsgCBFunc)&myHandle1::processMsg2
		);
	}

};
class MyServer1 :public HBBService
{
public:
	// 采用工厂模式，每次有新的连接建立，返回一个 HBBLogHandle1 对象与新的连接通信
	HBBServiceHandle *CreateServiceHandle() override
	{
		return new myHandle1();
	}
};
int main()
{
	std::cout << "HBBLOG SERVER" << std::endl;
	// 初始化日志服务
	int server_port = 18888; // 这个服务在 1888号端口上运行
	InitLog("127.0.0.1", LOG_PORT, server_port, SERVER1); // 连接日志中心，不启动的话本地控制台打印
	// 向注册中心注册
	myHandle1::RegMsgCallback(); //  注册消息回调
	MyServer1 server;
	server.set_arg(server_port,SERVER1,"127.0.0.1",REGISTER_PORT); // 向注册中心注册,第三个参数为注册中心地址，假设在本地
	server.Start(); // 开始监听
	while (1);
	return 0;
}

```

