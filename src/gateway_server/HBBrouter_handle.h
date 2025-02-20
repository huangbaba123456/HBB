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
    //���ӶϿ�����ʱ���������
	virtual void set_serverclient(HBBServiceProxyClient *serverclient) {
		this->serverclient_ = serverclient;
	}
	HBBServiceProxyClient* serverclient() {
		return serverclient_;
	}
	virtual void Close() override {
		// �ر�����
		if (serverclient_) {
			serverclient_->set_auto_delete(true);
			//auto tmp = serverclient_;
			serverclient_->set_xrouterhandle(NULL);
			serverclient_->Close(); // ɾ���Է�
			serverclient_ = NULL;
		}
		HBBServiceHandle::Close(); // �����ɾ������
	}
private:
	HBBServiceProxyClient *serverclient_=0;

};

#endif