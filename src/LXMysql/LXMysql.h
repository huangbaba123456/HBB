#ifndef LXMYSQL_H
#define LXMYSQL_H

#include  <vector>
#include  "LXData.h"
#include <mysql/mysql.h>
namespace LX{

//���к��������ܱ�֤�̰߳�ȫ
class LXAPI LXMysql
{
public:
	//��ʼ��Mysql API
	bool Init();

	//����ռ�õ�������Դ
	void Close();

	//���ݿ����ӣ��������̰߳�ȫ�� flag����֧�ֶ������
	bool Connect(const char*host, const char*user, const char*pass, const char*db, unsigned short port=3306, unsigned long flag=0);

	//ִ��sql���  if sqllen=0 strlen��ȡ�ַ�����
	bool Query(const char*sql, unsigned long sqllen = 0);

	//Mysql�������趨 Connect֮ǰ����
	bool Options(LX_OPT opt,const void *arg);

	//���ӳ�ʱʱ��
	bool SetConnectTimeout(int sec);

	//�Զ�������Ĭ�ϲ��Զ�
	bool SetReconnect(bool isre = true);

	//�������ȡ
	//����ȫ�����
	bool StoreResult();

	//��ʼ���ս����ͨ��Fetch��ȡ
	bool UseResult();

	//�ͷŽ����ռ�õĿռ�
	void FreeResult();

	//��ȡһ������
	std::vector<LXData> FetchRow();

	//����insert sql���
	std::string GetInsertSql(XDATA kv,std::string table);

	//����Ƕ��������� �ֶ�����ǰ��@ ���� @time �������ݲ������ţ�һ�����ڵ��ù��ܺ���
	bool Insert(XDATA kv, std::string table);

	//�������������
	bool InsertBin(XDATA kv, std::string table);

	//��ȡ�������ݵ�sql��� where����У��û�Ҫ����where
	std::string GetUpdateSql(XDATA kv, std::string table,std::string where);
	//���ظ���������ʧ�ܷ���-1
	int Update(XDATA kv, std::string table, std::string where);
	int UpdateBin(XDATA kv, std::string table, std::string where);

	//����ӿ�
	bool StartTransaction();
	bool StopTransaction();
	bool Commit();
	bool Rollback();

	//���׽ӿ�,����select�����ݽ����ÿ�ε���������һ�εĽ����
	XROWS GetResult(const char *sql);

	int GetInsertID();

protected:
	//mysql������
	MYSQL *mysql = 0;

	//�����
	MYSQL_RES *result = 0;

	//�ֶ����ƺ�����
	//std::vector<LXData> cols;
};

}

#endif