#include "HBBmsg.pb.h"
#include "HBBmsg_event.h"
#include "HBBtools.h"
#include <iostream>
#include <sstream>
#include <map>

#include "HBBlog_client.h"

using namespace std;
using namespace HBBmsg;
using namespace google;
using namespace protobuf;

//同一个类型只能有一个回调函数
static map< MsgType, HBBMsgEvent::MsgCBFunc> msg_callback;
void HBBMsgEvent::RegCB(HBBmsg::MsgType type, HBBMsgEvent::MsgCBFunc func)
{
    if (msg_callback.find(type) != msg_callback.end())
    {
        stringstream ss;
        ss << "RegCB is error," << type << " have been set " << endl;
        LOGERROR(ss.str().c_str());
        return;
    }
    msg_callback[type] = func;
}

void HBBMsgEvent::ReadCB(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    
    //回调消息函数
    auto ptr = msg_callback.find(head->msg_type());
    if (ptr == msg_callback.end())
    {
        Clear();
        LOGDEBUG("msg error func not set!");
        return;
    }
	// 这个时候我们存储
    auto func = ptr->second;
    (this->*func)(pb_head_, msg);
}
////接收消息，分发消息
void HBBMsgEvent::ReadCB()
{
    //如果线程退出
    while (1)
    {
        if (!RecvMsg())
        {
            Clear();
            return;
        }
        auto msg = GetMsg();
        if (!msg)return;
        //cout << "service_name = " << pb_head_->service_name() << endl;
        ReadCB(pb_head_, msg);
        Clear();
		if (is_drop_) {
			set_auto_delete(true);
			Close();
			return;
		}
    }
}
//////////////////////////////////////////
/// 接收数据包，
/// 1 正确接收到消息  (调用消息处理函数)
/// 2 消息接收不完整 (等待下一次接收) 
/// 3 消息接收出错 （退出清理空间）
/// @return 1 2 返回true 3返回false
bool HBBMsgEvent::RecvMsg()
{
    //解包
    
    //接收消息头
    if (!head_.size)
    {
        //1 消息头大小
        int len = Read(&head_.size, sizeof(head_.size));//bufferevent_read(bev_, &head_.size, sizeof(head_.size));
        if (len <= 0 || head_.size <= 0)
        {
            return false;
        }

        //分配消息头空间 读取消息头（鉴权，消息大小）
        if (!head_.Alloc(head_.size))
        {
            cerr << "head_.Alloc failed! Alloc size: " <<head_.size<< endl;
            return false;
        }
    }
    //2 开始接收消息头（鉴权，消息大小）
    if (!head_.Recved())
    {
        int len = Read(
            head_.data + head_.recv_size,  //第二次进来 从上次的位置开始读
            head_.size - head_.recv_size
        );
        if (len <= 0)
        {
            return true;
        }
        head_.recv_size += len;
        if (!head_.Recved())
            return true;

        //完整的头部数据接收完成
        //反序列化
        if (!pb_head_)
        {
            pb_head_ = new HBBMsgHead();
        }
        if (!pb_head_->ParseFromArray(head_.data, head_.size))
        {
            cerr << "pb_head.ParseFromArray failed!" << endl;
            return false;
        }
        
		if (pb_head_->msg_size() == 0) {
			// 空包数据，比如心跳包
			msg_.isRecv = 1;
			msg_.type = pb_head_->msg_type();
			msg_.size = 0;
			return true;
		}
		else {
			// 飞空数据包
			if (!msg_.Alloc(pb_head_->msg_size()))
			{
				cerr << "msg_.Alloc failed!  Alloc size: " << pb_head_->msg_size() << endl;
				return false;
			}
		}
        msg_.type = pb_head_->msg_type();
    }

    //3 开始接收消息内容
    if (!msg_.Recved())
    {
        int len = Read(
            msg_.data + msg_.recv_size,  //第二次进来 从上次的位置开始读
            msg_.size - msg_.recv_size
        );
        if (len <= 0)
        {
            return true;
        }
        msg_.recv_size += len;
    }
    return true;
}

/////////////////////////////////////////
/// 获取接收到的数据包，（不包含头部消息）
/// 由调用者清理HBBMsg
/// @return 如果没有完整的数据包，返回NULL
Msg *HBBMsgEvent::GetMsg()
{
    if (msg_.Recved())
        return &msg_;
    return NULL;
}
void HBBMsgEvent::Close()
{
    Clear();
    HBBComTask::Close();
}
/////////////////////////////////////
/// 清理缓存消息头和消息内容，用于接收下一次消息
void HBBMsgEvent::Clear()
{
    head_.Clear();
    msg_.Clear();
}
bool  HBBMsgEvent::SendMsg(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    if (!head || !msg)
        return false;
    head->set_msg_size(msg->size);
    //消息头序列化
    string head_str = head->SerializeAsString();
    int headsize = head_str.size();
    //stringstream ss;
    //ss << "SendMsg" << head->msg_type();
    //LOGDEBUG(ss.str());

    //1 发送消息头大小 4字节 暂时不考虑字节序问题
    int re = Write(&headsize, sizeof(headsize));
    if (!re)return false;

    //2 发送消息头（pb序列化） HBBMsgHead （设置消息内容的大小）
    re = Write(head_str.data(), head_str.size());
    if (!re)return false;

    re = Write(msg->data, msg->size);
    if (!re)return false;
    return true;
}

bool HBBMsgEvent::SendMsg(HBBmsg::HBBMsgHead *head, const Message *message)
{
    if (!message || !head)
        return false;
    ///封包
    //消息内容序列化
    string msg_str = message->SerializeAsString();
    int msg_size = msg_str.size();
	Msg msg;
    msg.data = (char*)msg_str.data();
    msg.size = msg_size;
    return SendMsg(head, &msg);
}
bool HBBMsgEvent::SendMsg(MsgType type, const Message *message)
{
    if (!message)
        return false;
    HBBMsgHead head;
    head.set_msg_type(type);
    return SendMsg(&head, message);
}
