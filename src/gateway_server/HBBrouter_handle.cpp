#include "HBBrouter_handle.h"
#include "HBBtools.h"
#include "HBBservice_proxy.h"
#include "HBBmsg.pb.h"
#include "HBBlog_client.h"
using namespace std;
using namespace HBBmsg;
void HBBRouterHandle::ReadCB(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    //转发消息
    bool ok=HBBServiceProxy::Get()->SendMsg(head, msg,this);
	if (!ok) {
		LOGDEBUG("send msg error");
	}
}

HBBRouterHandle::HBBRouterHandle()
{
}


HBBRouterHandle::~HBBRouterHandle()
{
}
