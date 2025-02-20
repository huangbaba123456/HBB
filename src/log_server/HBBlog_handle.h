#ifndef HBBLOGIN_HANDLE_H_
#define HBBLOGIN_HANDLE_H_
#include "HBBservice_handle.h"


class HBBLogHandle:public HBBServiceHandle
{

public:
	HBBLogHandle()
	{
		RegMsgCallback();
	}
	void AddLogReq(HBBmsg::HBBMsgHead* head, Msg* msg);
	static void RegMsgCallback() {
		RegCB(
			HBBmsg::MSG_ADD_LOG_REQ
			,(MsgCBFunc)&HBBLogHandle::AddLogReq
		);
	}

};
#endif

