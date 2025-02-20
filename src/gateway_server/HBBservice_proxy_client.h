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

    //发送数据，添加标识
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
		// 连接回调的时候，把队列里面的所有消息发送给对面
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
    //消息转发的对象，一个proxy对应多个XMsgEvent
    //用指针的值作为索引，要兼容64位
    //std::map<long long, XMsgEvent *> callback_task_;
    //std::mutex callback_task_mutex_;
	HBBRouterHandle *hbbrouterhandle_;
	std::list<HBBmsg::HBBMsgHead> headQue;
	std::list<Msg> msgQue;
};

#endif

