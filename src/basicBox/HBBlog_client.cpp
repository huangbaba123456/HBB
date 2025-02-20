#include "HBBlog_client.h"
#include "HBBtools.h"
#include <string>
#define LOG_LIST_MAX 100
using namespace std;
using namespace HBBmsg;


namespace HBBms {
	void HBBLog(
		HBBmsg::HBBLogLevel level
		,std::string msg
		,const char* filename
		,int line
	) {

		if (HBBLogClient::Get()->is_connected()) {
			// 如果连接成功了就写 发送会给数据库
			//cout << endl << endl;
			//cout <<"filename: " <<filename << endl;
			//cout << endl << endl;

			HBBAddLogReq req;
			req.set_log_level(level);
			req.set_log_txt(msg);
			req.set_filename(filename);
			req.set_line(line);
			
			HBBLogClient::Get()->AddLog(&req);
		}else {
			switch (level)
			{
			case HBBmsg::XLOG_DEBUG:{
				cout << "DEBUG" << ":" << filename << ":" << line << "\n" << msg << endl;
				break;
			}
			case HBBmsg::XLOG_INFO: {
				cout << "INFO" << ":" << filename << ":" << line << "\n" << msg << endl;
				break;
			}
			case HBBmsg::XLOG_ERROR: {
				cout << "ERROR" << ":" << filename << ":" << line << "\n" << msg << endl;
				break;
			}
			case HBBmsg::XLOG_FATAL: {
				cout << "FATAL" << ":" << filename << ":" << line << "\n" << msg << endl;
				break;
			}
			default:
				break;
			}
		}
	}
}
bool InitLog(std::string log_ip, int log_port, int my_port, std::string my_name) {
	// 初始化日志服务
	cout << "INIT LOG SERVER" << endl;
	HBBLogClient::Get()->set_server_ip(log_ip.c_str());
	HBBLogClient::Get()->set_server_port(log_port);
	HBBLogClient::Get()->set_my_name(my_name);
	HBBLogClient::Get()->set_my_port(my_port);
	HBBLogClient::Get()->set_auto_connect(true);
	HBBLogClient::Get()->set_timer_ms(100); // 设置定时器
	HBBLogClient::Get()->StartConnect(); // 开始连接
	return true;
}
void HBBLogClient::AddLog(const HBBmsg::HBBAddLogReq *req) {
	if (!req) {
		return;
	}
	if (req->log_level() < log_level_) {
		// 低水平消息不处理
		return;
	}
	string level_str = "Debug";
	switch (req->log_level())
	{
	case XLOG_DEBUG:
		level_str = "DEBUG";
		break;
	case XLOG_INFO:
		level_str = "INFO";
		break;
	case XLOG_ERROR:
		level_str = "ERROR";
		break;
	case XLOG_FATAL:
		level_str = "FATAL";
		break;
	default:
		break;
	}
	string log_time = HBBGetTime(0, "%F %T");
	stringstream log_text;
	log_text << "=========================================================\n";
	log_text << log_time << " " << level_str << "|" << req->filename() << ":" << req->line() << "\n";
	log_text << req->log_txt() << "\n";
	if (log_ofs_ && log_ofs_.is_open()) {
		log_ofs_.write(log_text.str().c_str(),log_text.str().size());
	}
	HBBAddLogReq tmp;
	tmp.CopyFrom(*req);
	if (tmp.log_time() <= 0) {
		tmp.set_log_time(time(0));
	}
	tmp.set_service_port(my_port_);
	tmp.set_service_name(my_name_);
	{
		HBBMutex mtx(&log_mutex_);
		if (logs_.size() > LOG_LIST_MAX) {
			logs_.pop_front();
		}
		logs_.push_back(tmp);
	}
}

void HBBLogClient::TimerCB() {
	while (1)
	{
		HBBAddLogReq log;
		{
			HBBMutex mutex(&log_mutex_);
			if (logs_.empty()) {
				break;
			}
			log = logs_.front();
			logs_.pop_front();
		}
		SendMsg(MSG_ADD_LOG_REQ,&log);
	}
}

