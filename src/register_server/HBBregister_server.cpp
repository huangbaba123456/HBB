#include "HBBregister_server.h"
#include "HBBregister_handle.h"
///���ݲ��� ��ʼ��������Ҫ�ȵ���
void HBBRegisterServer::main(int port)
{
    //ע����Ϣ�ص�����
    HBBRegisterHandle::RegMsgCallback();
    //���÷����������˿�
    set_server_port(port);
}
HBBServiceHandle* HBBRegisterServer::CreateServiceHandle()
{
    auto handle= new HBBRegisterHandle();
	handle->set_read_timeout_ms(5000);
	return handle;
}

//�ȴ��߳��˳�
void HBBRegisterServer::Wait()
{
    HBBThreadPool::Wait();
}