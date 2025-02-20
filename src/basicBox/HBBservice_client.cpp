#include "HBBmsg.pb.h"

#include "HBBservice_client.h"
#include "HBBlog_client.h"

using namespace std;
using namespace HBBmsg;
void HBBServiceClient::StartConnect()
{
    thread_pool_->Init(1);
    thread_pool_->Dispatch(this);
    //客户端不自动销毁，需要重连
    set_auto_delete(false);
}

HBBServiceClient::HBBServiceClient()
{
	this->thread_pool_ = HBBThreadPoolFactory::Create();
}
HBBServiceClient::~HBBServiceClient()
{
    //delete thread_pool_;
    //thread_pool_ = NULL;
}

HBBmsg::HBBMsgHead* HBBServiceClient::SetHead(HBBmsg::HBBMsgHead* head) {
	if (!head) {
		return NULL;
	}
	// 一定要设置service_name
	if (!service_name_.empty() && head->service_name().empty()) {
		head->set_service_name(service_name_);
	}
	// 下面设置一个登陆信息
	{
		// 鉴全
		HBBMutex mtx(&login_mtx);
		//head->set_token(login.token());
		//head->set_username(login.username());
		//head->set_rolename(login.rolename());
	}
	return head;

}

bool HBBServiceClient::SendMsg(HBBmsg::MsgType type, const google::protobuf::Message* message) {
	HBBMsgHead head;
	head.set_msg_type(type);

	SetHead(&head);
	return HBBMsgEvent::SendMsg(&head,message);
	
}