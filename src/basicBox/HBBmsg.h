#ifndef HBBMSG_H_
#define HBBMSG_H_
#include "HBBmsg.pb.h"
#include "HBBmsg.pb.h"//头部消息的最大字节数
#define MAX_MSG_SIZE 200000000






//所有的函数做内联
class Msg
{
public:
    //数据大小
    int size = 0;

    //消息类型
    HBBmsg::MsgType type = HBBmsg::NONE_DO_NOT_USE;

    //数据存放（protobuf的序列化后的数据）
    char *data = 0;

    //已经接收的数据大小
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

    //判断数据是否接收完成
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