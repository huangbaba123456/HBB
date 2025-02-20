#include "HBBlog_dao.h"
#include "HBBmsg.pb.h"
#include "LXMysql.h"
#include "HBBtools.h"
#include <thread>
#include <mutex>

using namespace std;
using namespace LX;
using namespace HBBmsg;
static LXMysql my;
static mutex log_mutex;

bool HBBLogDao::Init(const char *ip
	, const char *user
	, const char*pass
	, const char*db_name
	, int port ) {
	HBBMutex mtx(&log_mutex);
	if (!my.Init()) {
		return false;
	}
	my.SetReconnect(true); // 设置自动重新连接
	my.SetConnectTimeout(3);
	if (!my.Connect(ip, user, pass, db_name, port))
	{
		return false;
	}
	return true;
}
bool HBBLogDao::Install() {
	string sql = "";
	string table_name = "xms_log";

	//如果表不存在则创建
	sql = "CREATE TABLE IF NOT EXISTS `" + table_name + "` ( \
        `id` INT AUTO_INCREMENT,\
        `service_name` VARCHAR(16) ,\
        `service_port` INT ,\
        `service_ip` VARCHAR(16) ,\
        `log_txt` VARCHAR(4096) ,\
        `log_time` INT ,\
        `log_level` INT ,\
        PRIMARY KEY(`id`));";
	HBBMutex mux(&log_mutex);

	if (!my.Query(sql.c_str()))
	{
		return false;
	}
	return true;
}
bool HBBLogDao::AddLog(const HBBmsg::HBBAddLogReq *req) {
	if (!req){
		return false;
	}
	XDATA data;
	data["service_name"] = req->service_name().c_str();
	data["service_ip"] = req->service_ip().c_str();
	int service_port = req->service_port();
	data["service_port"] = &service_port;
	data["log_txt"] = req->log_txt().c_str();
	int log_time = req->log_time();
	data["log_time"] = &log_time;
	int log_level = req->log_level();
	data["log_level"] = &log_level;
	{
		HBBMutex mux(&log_mutex);
		return my.InsertBin(data, "xms_log");
	}
}