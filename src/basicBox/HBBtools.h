#ifndef HBBTOOLS_H
#define HBBTOOLS_H

#pragma once
// 头文件内容

#include "HBBmsg.pb.h"

#include <string>
#include <iostream>
#include <mutex>
#include <sstream>
#include <list>
#include <openssl/aes.h>
enum HBBMS_LOG_LEVEL
{
	HBBMS_LOG_LEVEL_DEBUG,
	HBBMS_LOG_LEVEL_INFO,
	HBBMS_LOG_LEVEL_ERROR
};
#define CUR_LEVEL  HBBMS_LOG_LEVEL_INFO

// 初始化日志服务的函数

bool InitLog(std::string log_ip, int log_port,int my_port,std::string my_name);


//#define LOG(level,msg) std::cout<<level<<":"<<__FILE__<<":"<<__LINE__<<"\n"<<msg<<std::endl;
//#define LOGDEBUG(msg) LOG("DEBUG",msg);
//#define LOGINFO(msg) LOG("INFO",msg);
//#define LOGERROR(msg) LOG("ERROR",msg);

 std::string GetDirData(std::string path);

 unsigned char *HBBMD5(const unsigned char *d, unsigned long n, unsigned char *md);



char *HBBMD5_base64(const unsigned char *d, unsigned long n, char *md);
std::string HBBMD5_base64(const unsigned char *d, unsigned long n);

bool Base64Encode(const unsigned char *in, int len, char *out_base64);
int Base64Decode(const char *in, int len, unsigned char *out_data);

std::string HBBGetTime(int timestamp, std::string fmt);


std::string HBBGetHostByName(std::string host_name);

struct HBBToolFileInfo
{
	std::string filename = "";
	long long filesize = 0;
	bool is_dir = false;
	long long time_write = 0;
	std::string time_str = "";// 2020-03-15 20:00:15
};


 std::list<HBBToolFileInfo> GetDirList(std::string path);
class HBBMutex
{
public:
    HBBMutex(std::mutex *mux)
    {
        mux_ = mux;
        mux->lock();
    }
    ~HBBMutex()
    {
        mux_->unlock();
    }
    std::mutex *mux_ = 0;
};

 std::string HBBGetIconFilename(std::string filename, bool is_dir);
 std::string HBBGetSizeString(long long size);

 void HBBNewDir(std::string path);
 void HBBDelFile(std::string path);

 long long GetDirSize(const char * path);


 bool GetDiskSize(const char *dir, unsigned long long *avail, unsigned long long *total, unsigned long long *free);

 class HBBAES{
public:
	static HBBAES * Create() {
		return new HBBAES();
	}
	bool SetKey(const char* key,int key_size,bool is_enc) {
		if (key_size > 32 || key_size <= 0) {
			std::cerr << "key_size > 32 || key_size <= 0: key_size= " << key_size << std::endl;
			return false;
		}
		unsigned char aes_key[32] = { 0 };
		memcpy(aes_key,key,key_size);
		int bit_size = 0;
		if (key_size > 24) {
			bit_size = 32 * 8;
		}else if (key_size > 16) {
			bit_size = 24 * 8;
		}else{
			bit_size = 16 * 8;
		}
		if (is_enc) {
			// 加密秘钥
			is_set_encode_ = true;
			if (AES_set_encrypt_key(aes_key,bit_size,&aes_)<0) {
				return false;
			}
			return true;
		}else {
			is_set_decode_ = true;
			if (AES_set_decrypt_key(aes_key,bit_size,&aes_)<0) {
				return false;
			}
			return true;
		}
	}
	void Drop() {
		delete this;
	}
	long long Encrypt(const unsigned char* in, long long in_size, unsigned char* out) {
		if (!in || in_size <= 0 || !out) {
			std::cerr << "Encrypt input data error" << std::endl;
			return 0;
		}
		if (!is_set_encode_) {
			std::cerr<<("is_set_encode_ is false");
			return 0;
		}
		long long  enc_byte = 0;
		unsigned char* p_in = 0;
		unsigned char* p_out = 0;
		unsigned char data[16] = { 0 };
		for (int i = 0; i < in_size; i += 16) {
			p_in = (unsigned char*)in + i;
			p_out = out + i;
			if (in_size - i < 16) {
				memcpy(data,p_in,in_size-i); // 最后不满16字节，补0
				p_in = data;
			}
			enc_byte += 16;
			AES_encrypt(p_in,p_out,&aes_);
		}
		return enc_byte;
	}
	long long Decrypt(const unsigned char* in, long long in_size, unsigned char* out) {
		if (!in || in_size <= 0 || !out || in_size % 16 != 0) {
			std::cerr << "Decrypt input data error" << std::endl;
			return 0;
		}
		if (!is_set_decode_) {
			std::cerr<<("is_set_decode_ is false") << std::endl;
			return 0;
		}
		long long enc_byte = 0;
		unsigned char* p_in = 0;
		unsigned char* p_out = 0;
		unsigned char data[16] = { 0 };
		for (int i = 0; i < in_size; i += 16) {
			p_in = (unsigned char*)in + i;
			p_out = out + i;
			enc_byte += 16;
			AES_decrypt(p_in,p_out,&aes_);
		}
		return enc_byte;
	}

private:
	AES_KEY aes_;
	bool is_set_decode_ = false;
	bool is_set_encode_ = false;
};
#endif // HBBTOOLS_H
