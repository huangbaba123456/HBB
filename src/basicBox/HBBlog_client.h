
#ifndef  HBBLOGCLIENT_H_
#define HBBLOGCLIENT_H_
#include "HBBmsg.pb.h"
#include "HBBservice_client.h"
#include <fstream>
#include <list>


namespace HBBms
{
	void HBBLog(HBBmsg::HBBLogLevel level, std::string msg, const char *filename, int line);
}
#define LOGDEBUG(msg) HBBms::HBBLog(HBBmsg::XLOG_DEBUG,msg,__FILE__,__LINE__);
#define LOGINFO(msg) HBBms::HBBLog(HBBmsg::XLOG_INFO,msg,__FILE__,__LINE__);
#define LOGERROR(msg) HBBms::HBBLog(HBBmsg::XLOG_ERROR,msg,__FILE__,__LINE__);
#define LOGFATAL(msg) HBBms::HBBLog(HBBmsg::XLOG_FATAL,msg,__FILE__,__LINE__);
bool InitLog(std::string log_ip, int log_port, int my_port, std::string my_name);
class HBBLogClient:public HBBServiceClient
{
public:
	static HBBLogClient* Get()
	{
		static HBBLogClient HBBc;
		return &HBBc;
	}
	virtual ~HBBLogClient(){}
	void AddLog(const HBBmsg::HBBAddLogReq* req);
	void set_log_level(HBBmsg::HBBLogLevel log_level) { 
		log_level_ = log_level;
	}
	void set_local_file(std::string local_file){
		log_ofs_.open(local_file);
	}
	void TimerCB() override;
	void set_my_port(int port) { my_port_ = port; }
	void set_my_name(std::string my_name) { my_name_ = my_name; }
private:
	HBBmsg::HBBLogLevel log_level_ = HBBmsg::XLOG_INFO;
	std::ofstream log_ofs_;
	std::list<HBBmsg::HBBAddLogReq> logs_;
	std::mutex log_mutex_;
	int my_port_ = 0;
	std::string my_name_ = "";


	HBBLogClient() {
	}
};


#endif // ! XLOGCLIENT_H_



