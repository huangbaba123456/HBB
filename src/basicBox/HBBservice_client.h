#ifndef HBBSERVICES_CLIENT_H

#define HBBSERVICES_CLIENT_H
#include "HBBmsg.pb.h"

#include "HBBmsg_event.h"
#include "HBBthread_pool.h"
#include <mutex>
#include "HBBtools.h"

class  HBBServiceClient :public HBBMsgEvent
{
public:
    HBBServiceClient();
    ~HBBServiceClient();
    //////////////////////////////////////////////////
    /// 将任务加入到线程池中，进行连接
    virtual void StartConnect();
	virtual void set_login(HBBmsg::HBBLoginRes* login) {
		if (!login) {
			return;
		}
		HBBMutex mtx(&login_mtx);
		this->login.CopyFrom(*login);
	}
	void set_service_name(const std::string& service_name) {
		this->service_name_ = service_name;
	}
	HBBmsg::HBBMsgHead* SetHead(HBBmsg::HBBMsgHead* head);

	virtual bool  SendMsg(HBBmsg::MsgType type,const google::protobuf::Message* message) override;
protected:
	std::mutex login_mtx;
	HBBmsg::HBBLoginRes login; // 没有初始化呀

	std::string service_name_; // 微服务的名词
    HBBThreadPool *thread_pool_ = 0;
};


#endif