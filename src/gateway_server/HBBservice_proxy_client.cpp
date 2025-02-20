#include "HBBservice_proxy_client.h"
#include "HBBtools.h"
#include "HBBrouter_handle.h"
#include "HBBlog_client.h"

using namespace std;
void HBBServiceProxyClient::ReadCB(HBBmsg::HBBMsgHead *head, Msg *msg)
{
	LOGDEBUG("HBBServiceProxyClient::ReadCB(HBBmsg::HBBMsgHead *head, HBBMsg *msg)");
    if (!head || !msg)return;
	if (hbbrouterhandle_) {
		hbbrouterhandle_->SendMsg(head, msg);
		LOGDEBUG("HBBrouterhandle_->SendMsg(head, msg);");
	}
	else {
		LOGDEBUG("not use HBBrouterhandle_->SendMsg(head, msg);");
	}
}

HBBServiceProxyClient::HBBServiceProxyClient()
{
}


HBBServiceProxyClient::~HBBServiceProxyClient()
{
}

HBBServiceProxyClient* HBBServiceProxyClient::Create(std::string service_name) {
	return new HBBServiceProxyClient();
}

void HBBServiceProxyClient::Close()  {
	// 重写关闭函数
	if (hbbrouterhandle_) {
		hbbrouterhandle_->set_serverclient(NULL);
		hbbrouterhandle_->Close(); // 关闭他的连接
	}
	HBBServiceClient::Close(); // 删除
}