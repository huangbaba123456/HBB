#include "HBBrouter_server.h"
#include "HBBrouter_handle.h"
#include "HBBssl_ctx.h"
#include "HBBlog_client.h"
using namespace std;
// 这个代码其实是有问题的
HBBServiceHandle* HBBRouterServer::CreateServiceHandle()
{

	HBBRouterHandle* handle=new HBBRouterHandle();
	//handle->set_ssl_ctx(ssl_ctx());
	return handle;
}