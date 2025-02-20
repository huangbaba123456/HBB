#ifndef HBBREGISTER_HANDLE_H_
#define HBBREGISTER_HANDLE_H_

#include "HBBservice_handle.h"
//////////////////////////
///����ע�����ĵĿͻ��� ��Ӧһ������
class HBBRegisterHandle :public HBBServiceHandle
{
public:
	HBBRegisterHandle();
	~HBBRegisterHandle();

	//���շ����ע������
	void RegisterReq(HBBmsg::HBBMsgHead *head, Msg *msg);


	//���շ���ķ�������
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


