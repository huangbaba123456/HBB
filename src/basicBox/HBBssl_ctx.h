#ifndef  HBBSSL_CXT_H_
#define HBBSSL_CXT_H_
#include "HBBssl.h"

class  HBBSSLCtx
{
public:

	/////////////////////////////////////////////////////////////////////
	/// 初始化SSL服务端
	/// @crt_file 服务端证书文件
	/// @key_file 服务端私钥文件
	/// @ca_file 验证客户端证书（可选）
	/// @return 初始化是否成功
	virtual bool InitServer(const char*crt_file, const char *key_file, const char *ca_file = 0);

	/////////////////////////////////////////////////////////////////////
	/// 初始化SSL客户端
	/// @para ca_file 验证服务端证书
	virtual bool InitClient(const char *ca_file = 0);

	/////////////////////////////////////////////////////////////////////
	/// 创建SSL通信对象，socket和ssl_st资源由调用者释放
	/// 创建失败返回通过HBBSSL::IsEmpty()判断
	HBBSSL NewHBBSSL(int socket);

	//释放资源
	void Close();

	HBBSSLCtx();

	~HBBSSLCtx();
private:
	struct ssl_ctx_st *ssl_ctx_ = 0;
	//////////////////////////////////////////////////////////////////////
	/// 验证对方证书
	void SetVerify(const char *ca_crt);
};

#endif // ! HBBSSL_CXT_H_




