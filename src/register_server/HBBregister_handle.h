#ifndef HBBREGISTER_HANDLE_H_
#define HBBREGISTER_HANDLE_H_

#include "HBBservice_handle.h"
//////////////////////////
///处理注册中心的客户端 对应一个连接
class HBBRegisterHandle :public HBBServiceHandle
{
public:
	HBBRegisterHandle();
	~HBBRegisterHandle();

	//接收服务的注册请求
	void RegisterReq(HBBmsg::HBBMsgHead *head, Msg *msg);


	//接收服务的发现请求
	void GetServiceReq(HBBmsg::HBBMsgHead *head, Msg *msg);
	void HeartRes(HBBmsg::HBBMsgHead *head, Msg *msg) {};
	static void RegMsgCallback()
	{
		RegCB(HBBmsg::MSG_HEART_REQ, (MsgCBFunc)&HBBRegisterHandle::HeartRes);
		RegCB(HBBmsg::MSG_REGISTER_REQ, (MsgCBFunc)&HBBRegisterHandle::RegisterReq);
		RegCB(HBBmsg::MSG_GET_SERVICE_REQ, (MsgCBFunc)&HBBRegisterHandle::GetServiceReq);
	}
};




#endif // !HBBREGISTER_HANDLE_H_


