#include <iostream>
#include "HBBservice.h"
#include "HBBlog_handle.h"
#include "HBBlog_dao.h"
#include "HBBconfig.h"
using namespace std;

class HBBLogServer :public HBBService
{
public:
	HBBServiceHandle *CreateServiceHandle() override
	{
		return new HBBLogHandle();
	}
	void main(int port) {
		HBBLogHandle::RegMsgCallback();
		set_server_port(port);
	}

};
int main()
{
	std::cout << "HBBLOG SERVER" << std::endl;
	HBBLogDao::Get()->Init("localhost", "root", "114477", "testlxmysql", 3306);
	HBBLogDao::Get()->Install();
	HBBLogServer server;
	server.main(LOG_PORT);
	server.Start();
	HBBThreadPool::Wait();
	return 0;
}

