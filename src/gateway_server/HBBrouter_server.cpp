#include "HBBrouter_server.h"
#include "HBBrouter_handle.h"
#include "HBBssl_ctx.h"
#include "HBBlog_client.h"
using namespace std;
// ���������ʵ���������
HBBServiceHandle* HBBRouterServer::CreateServiceHandle()
{

	HBBRouterHandle* handle=new HBBRouterHandle();
	//handle->set_ssl_ctx(ssl_ctx());
	return handle;
}