
#ifndef HBBMSG_EVENT_H_
#define HBBMSG_EVENT_H_
#include "HBBtype.pb.h"
#include "HBBmsg.pb.h"

#include "HBBmsg.h"
#include "HBBcom_task.h"
class  HBBMsgEvent:public HBBComTask
{
public:

    ////������Ϣ���ַ���Ϣ
    virtual void ReadCB();
    //��Ϣ�ص�������Ĭ�Ϸ��͵��û�ע��ĺ�����·������
    virtual void ReadCB(HBBmsg::HBBMsgHead *head, Msg *msg);

    //////////////////////////////////////////
    /// �������ݰ���
    /// 1 ��ȷ���յ���Ϣ  (������Ϣ������)
    /// 2 ��Ϣ���ղ����� (�ȴ���һ�ν���) 
    /// 3 ��Ϣ���ճ��� ���˳�����ռ䣩
    /// @return 1 2 ����true 3����false
    bool RecvMsg();

    /////////////////////////////////////////
    /// ��ȡ���յ������ݰ�����������ͷ����Ϣ��
    /// �ɵ���������HBBMsg
    /// @return ���û�����������ݰ�������NULL
    Msg *GetMsg();

    //////////////////////////////////
    /// ������Ϣ������ͷ�����Զ�������
    /// @type ��Ϣ����
    /// @message ��Ϣ����
    /// @return ���ʹ��󣬱���bevδ����
    virtual bool  SendMsg(HBBmsg::MsgType type, const google::protobuf::Message *message);
    virtual bool  SendMsg(HBBmsg::HBBMsgHead *head, const google::protobuf::Message *message);
    virtual bool  SendMsg(HBBmsg::HBBMsgHead *head, Msg *msg);

    /////////////////////////////////////
    /// ��������Ϣͷ����Ϣ���ݣ����ڽ�����һ����Ϣ
    void Clear();

    void Close();

    typedef void (HBBMsgEvent::*MsgCBFunc) (HBBmsg::HBBMsgHead *head, Msg *msg);
    ////////////////////////////////////////////////////
    /// �����Ϣ����Ļص�������������Ϣ���ͷַ� ,ͬһ������ֻ����һ���ص�����
    /// @para type ��Ϣ����
    /// @para func ��Ϣ�ص�����
    static void RegCB(HBBmsg::MsgType type, MsgCBFunc func);
	virtual void DropInMsg() { is_drop_ = true; }
private:
	Msg head_; //��Ϣͷ
	Msg msg_;  //��Ϣ����
	bool is_drop_ = false;
    //pb��Ϣͷ
    HBBmsg::HBBMsgHead *pb_head_ = 0;
	
};

#endif