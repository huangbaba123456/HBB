#ifndef  HBBLOG_DAO_H_
#define HBBLOG_DAO_H_

namespace HBBmsg
{
	class HBBAddLogReq;
}

class HBBLogDao
{
public:
	static HBBLogDao *Get()
	{
		static HBBLogDao HBBd;
		return &HBBd;
	}
	~HBBLogDao() {}
	bool Init(
		const char *ip
		, const char *user
		, const char*pass
		, const char*db_name
		, int port = 3306
	);
	bool Install();
	bool AddLog(const HBBmsg::HBBAddLogReq *req);
private:
	HBBLogDao() {
	
	}
};

#endif // ! XLOG_DAO_H_




