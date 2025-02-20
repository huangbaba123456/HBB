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

//ͬһ������ֻ����һ���ص�����
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
    
    //�ص���Ϣ����
    auto ptr = msg_callback.find(head->msg_type());
    if (ptr == msg_callback.end())
    {
        Clear();
        LOGDEBUG("msg error func not set!");
        return;
    }
	// ���ʱ�����Ǵ洢
    auto func = ptr->second;
    (this->*func)(pb_head_, msg);
}
////������Ϣ���ַ���Ϣ
void HBBMsgEvent::ReadCB()
{
    //����߳��˳�
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
/// �������ݰ���
/// 1 ��ȷ���յ���Ϣ  (������Ϣ������)
/// 2 ��Ϣ���ղ����� (�ȴ���һ�ν���) 
/// 3 ��Ϣ���ճ��� ���˳�����ռ䣩
/// @return 1 2 ����true 3����false
bool HBBMsgEvent::RecvMsg()
{
    //���
    
    //������Ϣͷ
    if (!head_.size)
    {
        //1 ��Ϣͷ��С
        int len = Read(&head_.size, sizeof(head_.size));//bufferevent_read(bev_, &head_.size, sizeof(head_.size));
        if (len <= 0 || head_.size <= 0)
        {
            return false;
        }

        //������Ϣͷ�ռ� ��ȡ��Ϣͷ����Ȩ����Ϣ��С��
        if (!head_.Alloc(head_.size))
        {
            cerr << "head_.Alloc failed! Alloc size: " <<head_.size<< endl;
            return false;
        }
    }
    //2 ��ʼ������Ϣͷ����Ȩ����Ϣ��С��
    if (!head_.Recved())
    {
        int len = Read(
            head_.data + head_.recv_size,  //�ڶ��ν��� ���ϴε�λ�ÿ�ʼ��
            head_.size - head_.recv_size
        );
        if (len <= 0)
        {
            return true;
        }
        head_.recv_size += len;
        if (!head_.Recved())
            return true;

        //������ͷ�����ݽ������
        //�����л�
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
			// �հ����ݣ�����������
			msg_.isRecv = 1;
			msg_.type = pb_head_->msg_type();
			msg_.size = 0;
			return true;
		}
		else {
			// �ɿ����ݰ�
			if (!msg_.Alloc(pb_head_->msg_size()))
			{
				cerr << "msg_.Alloc failed!  Alloc size: " << pb_head_->msg_size() << endl;
				return false;
			}
		}
        msg_.type = pb_head_->msg_type();
    }

    //3 ��ʼ������Ϣ����
    if (!msg_.Recved())
    {
        int len = Read(
            msg_.data + msg_.recv_size,  //�ڶ��ν��� ���ϴε�λ�ÿ�ʼ��
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
/// ��ȡ���յ������ݰ�����������ͷ����Ϣ��
/// �ɵ���������HBBMsg
/// @return ���û�����������ݰ�������NULL
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
/// ��������Ϣͷ����Ϣ���ݣ����ڽ�����һ����Ϣ
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
    //��Ϣͷ���л�
    string head_str = head->SerializeAsString();
    int headsize = head_str.size();
    //stringstream ss;
    //ss << "SendMsg" << head->msg_type();
    //LOGDEBUG(ss.str());

    //1 ������Ϣͷ��С 4�ֽ� ��ʱ�������ֽ�������
    int re = Write(&headsize, sizeof(headsize));
    if (!re)return false;

    //2 ������Ϣͷ��pb���л��� HBBMsgHead ��������Ϣ���ݵĴ�С��
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
    ///���
    //��Ϣ�������л�
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
