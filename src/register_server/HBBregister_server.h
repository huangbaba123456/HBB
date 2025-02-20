#ifndef XREGISTERSERVER_H
#define XREGISTERSERVER_H
#include "HBBservice.h"

////////////////////////////////////
//// 注册中心服务端
class HBBRegisterServer:public HBBService
{
public:

    ///根据参数 初始化服务，需要先调用
    void main(int port);

    //等待线程退出
    void Wait();

    HBBServiceHandle* CreateServiceHandle();

};


#endif