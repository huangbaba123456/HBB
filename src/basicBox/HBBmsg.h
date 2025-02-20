#ifndef HBBMSG_H_
#define HBBMSG_H_
#include "HBBmsg.pb.h"
#include "HBBmsg.pb.h"//ͷ����Ϣ������ֽ���
#define MAX_MSG_SIZE 200000000






//���еĺ���������
class Msg
{
public:
    //���ݴ�С
    int size = 0;

    //��Ϣ����
    HBBmsg::MsgType type = HBBmsg::NONE_DO_NOT_USE;

    //���ݴ�ţ�protobuf�����л�������ݣ�
    char *data = 0;

    //�Ѿ����յ����ݴ�С
    int recv_size = 0;

	bool isRecv = 0;

	bool Alloc(int s)
    {
        if (s <= 0 || s > MAX_MSG_SIZE)
            return false;
        if (data)
            delete data;
        data = new char[s];
        if (!data)
            return false;
        this->size = s;
        this->recv_size = 0;
        return true;
    }

    //�ж������Ƿ�������
    bool Recved()
    {
		if (isRecv) {
			return true;
		}
        if (size <= 0)return false;
        return (recv_size == size);
    }

    void Clear()
    {
		if (data) {
			delete data;
			data = NULL;
		}
        memset(this, 0, sizeof(Msg));
    }
	~Msg() {

	}
	Msg() {
		
	}
	Msg(const Msg& msg) {
		this->size = msg.size;
		this->Alloc(this->size);
		memcpy(this->data,msg.data,msg.size);
	}
	Msg& operator=(const Msg& msg) {
		if (this == &msg) {
			return *this;
		}
		Clear();
		this->size = msg.size;
		this->Alloc(this->size);
		memcpy(this->data, msg.data, msg.size);
		return *this;
	}


};

#endif