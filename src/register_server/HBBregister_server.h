#ifndef XREGISTERSERVER_H
#define XREGISTERSERVER_H
#include "HBBservice.h"

////////////////////////////////////
//// ע�����ķ����
class HBBRegisterServer:public HBBService
{
public:

    ///���ݲ��� ��ʼ��������Ҫ�ȵ���
    void main(int port);

    //�ȴ��߳��˳�
    void Wait();

    HBBServiceHandle* CreateServiceHandle();

};


#endif