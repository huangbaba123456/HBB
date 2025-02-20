#ifndef HBBROUTER_HANDLE_H
#define HBBROUTER_HANDLE_H
#include "HBBservice_handle.h"
#include "HBBservice_proxy_client.h"
class HBBRouterHandle : public HBBServiceHandle
{
public:
    HBBRouterHandle();
    virtual ~HBBRouterHandle() ;
    virtual void ReadCB(HBBmsg::HBBMsgHead *head, Msg *msg) override;
    //连接断开，超时，出错调用
	virtual void set_serverclient(HBBServiceProxyClient *serverclient) {
		this->serverclient_ = serverclient;
	}
	HBBServiceProxyClient* serverclient() {
		return serverclient_;
	}
	virtual void Close() override {
		// 关闭连接
		if (serverclient_) {
			serverclient_->set_auto_delete(true);
			//auto tmp = serverclient_;
			serverclient_->set_xrouterhandle(NULL);
			serverclient_->Close(); // 删除对方
			serverclient_ = NULL;
		}
		HBBServiceHandle::Close(); // 父类的删除函数
	}
private:
	HBBServiceProxyClient *serverclient_=0;

};

#endif