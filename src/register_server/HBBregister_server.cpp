#include "HBBregister_server.h"
#include "HBBregister_handle.h"
///根据参数 初始化服务，需要先调用
void HBBRegisterServer::main(int port)
{
    //注册消息回调函数
    HBBRegisterHandle::RegMsgCallback();
    //设置服务器监听端口
    set_server_port(port);
}
HBBServiceHandle* HBBRegisterServer::CreateServiceHandle()
{
    auto handle= new HBBRegisterHandle();
	handle->set_read_timeout_ms(5000);
	return handle;
}

//等待线程退出
void HBBRegisterServer::Wait()
{
    HBBThreadPool::Wait();
}