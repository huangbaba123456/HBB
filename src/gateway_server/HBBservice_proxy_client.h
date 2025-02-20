#ifndef HBBSERVICEPROXYCLIENT
#define HBBSERVICEPROXYCLIENT
#include "HBBservice_client.h"
//#include "xrouter_handle.h"
#include <map>
#include <list>
class HBBRouterHandle;

class HBBServiceProxyClient :public HBBServiceClient
{
public:
	static HBBServiceProxyClient* Create(std::string service_name);
	HBBServiceProxyClient();

	~HBBServiceProxyClient();
    virtual void ReadCB(HBBmsg::HBBMsgHead *head, Msg *msg) override;

    //�������ݣ���ӱ�ʶ
    //virtual bool SendMsg(xmsg::XMsgHead *head, XMsg *msg);
	void set_xrouterhandle(HBBRouterHandle *hbbrouterhandle) {
		this->hbbrouterhandle_ = hbbrouterhandle;
	}
	HBBRouterHandle* hbbrouterhandle() {
		return hbbrouterhandle_;
	}
	void addMsg(HBBmsg::HBBMsgHead* head, Msg *msg) {
		HBBmsg::HBBMsgHead curHead;
		curHead.CopyFrom(*head);
		Msg curMsg = *msg;
		headQue.push_back(curHead);
		msgQue.push_back(curMsg);
	}
	virtual void ConnectedCB() override{
		// ���ӻص���ʱ�򣬰Ѷ��������������Ϣ���͸�����
		while (!msgQue.empty()){
			HBBmsg::HBBMsgHead curHead = headQue.front();
			Msg curMsg = msgQue.front();
			headQue.pop_front();
			msgQue.pop_front();
			HBBMsgEvent::SendMsg(&curHead,&curMsg);
			curMsg.Clear();
		}
	}
	virtual void Close() override;

private:
    //��Ϣת���Ķ���һ��proxy��Ӧ���XMsgEvent
    //��ָ���ֵ��Ϊ������Ҫ����64λ
    //std::map<long long, XMsgEvent *> callback_task_;
    //std::mutex callback_task_mutex_;
	HBBRouterHandle *hbbrouterhandle_;
	std::list<HBBmsg::HBBMsgHead> headQue;
	std::list<Msg> msgQue;
};

#endif

