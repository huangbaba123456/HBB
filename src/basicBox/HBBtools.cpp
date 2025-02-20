#include "HBBtools.h"
#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include "HBBmsg.pb.h"
#include "HBBlog_client.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/vfs.h>
#define _access access
#define _mkdir(d) mkdir(d,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

#endif

using namespace std;
using namespace HBBmsg;





 std::string HBBGetTime(int timestamp, std::string fmt)
{
	char time_buf[128] = { 0 };
	time_t tm = timestamp;
	if (timestamp <= 0)
		tm = time(0);
	strftime(time_buf, sizeof(time_buf), fmt.c_str(), gmtime(&tm));
	return time_buf;
}

 std::string HBBGetHostByName(std::string host_name) {
#ifdef  _WIN32
	static bool is_init = false;
	if(!is_init){
		is_init = true;
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			printf("WSAStartup failed\n");
			return "";
		}
	}
#endif // ! _WIN32

	auto host = gethostbyname(host_name.c_str());
	if (!host) {
		return "127.0.0.1";
	}
	auto addr = host->h_addr_list;
	if (!addr) {
		return "";
	}
	return inet_ntoa(*(	(in_addr*)*addr	));

}
 bool Base64Encode(const unsigned char *in, int len, char *out_base64)
{
	if (!in || len <= 0 || !out_base64)
		return 0;
	//源 内存
	auto mem_bio = BIO_new(BIO_s_mem());
	if (!mem_bio)return 0;

	//base64过滤器
	auto b64_bio = BIO_new(BIO_f_base64());
	if (!b64_bio)
	{
		BIO_free(mem_bio);
		return 0;
	}

	//64字节加不换行符"\n"
	BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);

	//形成BIO链
	// b64-mem
	BIO_push(b64_bio, mem_bio);

	// 往链表头部写入，base64过滤器处理后转入下一个节点（链尾）
	// 对链表写入会调用编码
	// 返回写入的数据大小
	int re = BIO_write(b64_bio, in, len);
	if (re <= 0)
	{
		//清理链表
		BIO_free_all(b64_bio);
		return 0;
	}
	//刷新缓存，写入链表的mem
	BIO_flush(b64_bio);

	BUF_MEM *p_data = 0;
	int out_size = 0;
	//从链表源内存中读取
	BIO_get_mem_ptr(b64_bio, &p_data);
	if (p_data)
	{
		memcpy(out_base64, p_data->data, p_data->length);
		out_size = p_data->length;
	}
	BIO_free_all(b64_bio);
	return out_size;
}

 int Base64Decode(const char *in, int len, unsigned char *out_data)
{
	if (!in || len <= 0 || !out_data)
		return 0;
	//内存源 (密文)
	auto mem_bio = BIO_new_mem_buf(in, len);
	if (!mem_bio)return 0;

	//base64过滤器
	auto b64_bio = BIO_new(BIO_f_base64());
	if (!b64_bio)
	{
		BIO_free(mem_bio);
		return 0;
	}

	//64字节加不换行符"\n"
	BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);

	//形成BIO链
	// b64-mem
	BIO_push(b64_bio, mem_bio);

	//读取解码后数据
	size_t size = 0;
	BIO_read_ex(b64_bio, out_data, len, &size);

	BIO_free_all(b64_bio);
	return size;
}

//生成md5 128bit(16字节) 
 unsigned char *HBBMD5(const unsigned char *d, unsigned long n, unsigned char *md)
{
	return MD5(d, n, md);
}
 std::string HBBMD5_base64(const unsigned char *d, unsigned long n)
{
	unsigned char buf[16] = { 0 };
	HBBMD5(d, n, buf);
	char base64[32] = { 0 };
	Base64Encode(buf, 16, base64);
	return base64;
}
//生成md5 128bit(16字节) 再经过base64转化为字符串
 char *HBBMD5_base64(const unsigned char *d, unsigned long n, char *md)
{
	unsigned char buf[16] = { 0 };
	HBBMD5(d, n, buf);
	Base64Encode(buf, 16, md);
	return md;
}
 std::string GetDirData(std::string path)
{
	string data = "";
#ifdef _WIN32
	//存储文件信息
	_finddata_t file;
	string dirpath = path + "/*.*";
	//目录上下文
	intptr_t dir = _findfirst(dirpath.c_str(), &file);
	if (dir < 0)
		return data;
	do
	{
		if (file.attrib & _A_SUBDIR) continue;
		char buf[1024] = { 0 };
		sprintf(buf, "%s,%u;", file.name, file.size);
		data += buf;
	} while (_findnext(dir, &file) == 0);
#else
	const char *dir = path.c_str();
	DIR *dp = 0;
	struct dirent *entry = 0;
	struct stat statbuf;
	dp = opendir(dir);
	if (dp == NULL)
		return data;
	chdir(dir);
	char buf[1024] = { 0 };
	while ((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode))continue;
		sprintf(buf, "%s,%ld;", entry->d_name, statbuf.st_size);
		data += buf;
	}
#endif
	//去掉结尾 ;
	if (!data.empty())
	{
		data = data.substr(0, data.size() - 1);
	}
	return data;
}

 std::list<HBBToolFileInfo> GetDirList(std::string path)
{
	std::list< HBBToolFileInfo > file_list;

#ifdef _WIN32
	//存储文件信息
	_finddata_t file;
	string dirpath = path + "/*.*";
	//目录上下文
	intptr_t dir = _findfirst(dirpath.c_str(), &file);
	if (dir < 0)
		return file_list;
	char time_buf[128] = { 0 };
	do
	{
		HBBToolFileInfo file_info;
		if (file.attrib & _A_SUBDIR)
		{
			file_info.is_dir = true;
		}
		file_info.filename = file.name;
		file_info.filesize = file.size;
		file_info.time_write = file.time_write;
		/*

		%a 星期几的简写
%A 星期几的全称
%b 月分的简写
%B 月份的全称
%c 标准的日期的时间串
%C 年份的后两位数字
%d 十进制表示的每月的第几天
%D 月/天/年
%e 在两字符域中，十进制表示的每月的第几天
%F 年-月-日
%g 年份的后两位数字，使用基于周的年
%G 年分，使用基于周的年
%h 简写的月份名
%H 24小时制的小时
%I 12小时制的小时
%j 十进制表示的每年的第几天
%m 十进制表示的月份
%M 十时制表示的分钟数
%n 新行符
%p 本地的AM或PM的等价显示
%r 12小时的时间
%R 显示小时和分钟：hh:mm
%S 十进制的秒数
%t 水平制表符
%T 显示时分秒：hh:mm:ss
%u 每周的第几天，星期一为第一天 （值从0到6，星期一为0）
%U 第年的第几周，把星期日做为第一天（值从0到53）
%V 每年的第几周，使用基于周的年
%w 十进制表示的星期几（值从0到6，星期天为0）
%W 每年的第几周，把星期一做为第一天（值从0到53）
%x 标准的日期串
%X 标准的时间串
%y 不带世纪的十进制年份（值从0到99）
%Y 带世纪部分的十制年份
%z，%Z 时区名称，如果不能得到时区名称则返回空字符。
%% 百分号下面的程序则显示当前的完整日期：

		*/
		time_t tm = file_info.time_write;
		strftime(time_buf, sizeof(time_buf), "%F %R", gmtime(&tm));
		file_info.time_str = time_buf;
		file_list.push_back(file_info);
	} while (_findnext(dir, &file) == 0);
	_findclose(dir);
#else
	const char *dir = path.c_str();
	DIR *dp = 0;
	struct dirent *entry = 0;
	struct stat statbuf;
	dp = opendir(dir);
	if (dp == NULL)
		return file_list;
	chdir(dir);
	char buf[1024] = { 0 };
	while ((entry = readdir(dp)) != NULL)
	{
		HBBToolFileInfo file_info;
		lstat(entry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode))
		{
			file_info.is_dir = true;
		}
		file_info.filename = entry->d_name;
		file_info.filesize = statbuf.st_size;
		/*
			   struct timespec st_atim;  // Time of last access
			   struct timespec st_mtim;  // Time of last modification
			   struct timespec st_ctim;  // Time of last status change

		   #define st_atime st_atim.tv_sec      // Backward compatibility
		   #define st_mtime st_mtim.tv_sec
		   #define st_ctime st_ctim.tv_sec
		*/
		file_info.time_write = statbuf.st_mtime;
		time_t tm = file_info.time_write;
		char time_buf[16] = { 0 };
		strftime(time_buf, sizeof(time_buf), "%F %R", gmtime(&tm));
		file_info.time_str = time_buf;
		file_list.push_back(file_info);
	}
	closedir(dp);
#endif

	return file_list;
}

 std::string HBBGetSizeString(long long size)
{
	string filesize_str = "";
	if (size > 1024 * 1024 * 1024) //GB
	{
		double gb_size = (double)size / (double)(1024 * 1024 * 1024);
		long long tmp = gb_size * 100;

		stringstream ss;
		ss << tmp / 100;
		if (tmp % 100 > 0)
			ss << "." << tmp % 100;
		ss << "GB";
		filesize_str = ss.str();
	}
	else if (size > 1024 * 1024) //MB
	{
		double gb_size = (double)size / (double)(1024 * 1024);
		long long tmp = gb_size * 100;

		stringstream ss;
		ss << tmp / 100;
		if (tmp % 100 > 0)
			ss << "." << tmp % 100;
		ss << "MB";
		filesize_str = ss.str();
	}
	else if (size > 1024) //KB
	{
		float gb_size = (float)size / (float)(1024);
		long long tmp = gb_size * 100;
		stringstream ss;
		ss << tmp / 100;
		if (tmp % 100 > 0)
			ss << "." << tmp % 100;
		ss << "KB";
		filesize_str = ss.str();
	}
	else //B
	{
		float gb_size = size / (float)(1024);
		long long tmp = gb_size * 100;

		stringstream ss;
		ss << size;
		ss << "B";
		filesize_str = ss.str();
	}
	return filesize_str;
}
 std::string HBBGetIconFilename(std::string filename, bool is_dir)
{
	string iconpath = "Other";
	///文件类型
	string filetype = "";
	int pos = filename.find_last_of('.');
	if (pos > 0)
	{
		filetype = filename.substr(pos + 1);
	}
	//转换为小写 ，第三个参数是输出
	transform(filetype.begin(), filetype.end(), filetype.begin(), ::tolower);

	if (is_dir)
	{
		iconpath = "Folder";
	}
	else if (filetype == "jpg" || filetype == "png" || filetype == "gif")
	{
		iconpath = "Img";
	}
	else if (filetype == "doc" || filetype == "docx" || filetype == "wps")
	{
		iconpath = "Doc";
	}
	else if (filetype == "rar" || filetype == "zip" || filetype == "7z" || filetype == "gzip")
	{
		iconpath = "Rar";
	}
	else if (filetype == "ppt" || filetype == "pptx")
	{
		iconpath = "Ppt";
	}
	else if (filetype == "xls" || filetype == "xlsx")
	{
		iconpath = "Xls";
	}
	else if (filetype == "pdf")
	{
		iconpath = "Pdf";
	}
	else if (filetype == "doc" || filetype == "docx" || filetype == "wps")
	{
		iconpath = "Doc";
	}
	else if (filetype == "avi" || filetype == "mp4" || filetype == "mov" || filetype == "wmv")
	{
		iconpath = "Video";
	}
	else if (filetype == "mp3" || filetype == "pcm" || filetype == "wav" || filetype == "wma")
	{
		iconpath = "Music";
	}
	else
	{
		iconpath = "Other";
	}
	return iconpath;
}



  std::string HBBFormatDir(const std::string &dir)
{
	std::string re = "";
	bool is_sep = false; // 是否是/ 或者是 "\"  \// 
	for (int i = 0; i < dir.size(); i++)
	{
		if (dir[i] == '/' || dir[i] == '\\')
		{
			if (is_sep)
			{
				continue;
			}
			re += '/';
			is_sep = true;
			continue;
		}
		is_sep = false;
		re += dir[i];
	}
	return re;
}

  void HBBStringSplit(std::vector<string> &vec, std::string str, std::string find)
{
	int pos1 = 0;
	int pos2 = 0;
	vec.clear();
	while ((pos2 = str.find(find, pos1)) != (int)string::npos)
	{
		vec.push_back(str.substr(pos1, pos2 - pos1));
		pos1 = pos2 + find.length();
	}
	string strTemp = str.substr(pos1);
	if ((int)strTemp.size() > 0)
	{
		vec.push_back(str.substr(pos1));
	}
}
 void HBBNewDir(std::string path)
{
	string tmp = HBBFormatDir(path);

	vector<string>paths;
	HBBStringSplit(paths, tmp, "/");

	string tmpstr = "";
	for (auto s : paths)
	{
		tmpstr += s + "/";
		if (_access(tmpstr.c_str(), 0) == -1)
		{
			_mkdir(tmpstr.c_str());
		}
	}
}

//XCOM_API void XDelFile(std::string path) {
//
//#ifdef  _WIN32
//	DeleteFileA(path.c_str());
//#else
//	remove(path.c_str());
//#endif //  _WIN32
//
//}

 void HBBDelFile(string path) {
#ifdef _WIN32
	// 检测路径是否是目录
	DWORD attributes = GetFileAttributesA(path.c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		std::cerr << "Path not found: " << path << std::endl;
		return;
	}

	if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
		// 是目录，递归删除
		WIN32_FIND_DATAA findFileData;
		HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findFileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			std::cerr << "Failed to list directory: " << path << std::endl;
			return;
		}

		do {
			std::string name = findFileData.cFileName;
			if (name == "." || name == "..") {
				continue;
			}
			std::string fullPath = path + "\\" + name;
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// 递归删除子目录
				HBBDelFile(fullPath);
			}
			else {
				// 删除文件
				if (!DeleteFileA(fullPath.c_str())) {
					std::cerr << "Failed to delete file: " << fullPath << std::endl;
				}
			}
		} while (FindNextFileA(hFind, &findFileData) != 0);

		FindClose(hFind);

		// 删除空目录
		if (!RemoveDirectoryA(path.c_str())) {
			std::cerr << "Failed to delete directory: " << path << std::endl;
		}
	}
	else {
		// 是文件，删除
		if (!DeleteFileA(path.c_str())) {
			std::cerr << "Failed to delete file: " << path << std::endl;
		}
	}
#else
	// POSIX 平台
	struct stat pathStat;
	if (stat(path.c_str(), &pathStat) == -1) {
		perror(("Path not found: " + path).c_str());
		return;
	}

	if (S_ISDIR(pathStat.st_mode)) {
		// 是目录，递归删除
		DIR* dir = opendir(path.c_str());
		if (!dir) {
			perror(("Failed to open directory: " + path).c_str());
			return;
		}

		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			std::string name = entry->d_name;
			if (name == "." || name == "..") {
				continue;
			}
			std::string fullPath = path + "/" + name;

			// 检查是否是子目录
			struct stat entryStat;
			if (stat(fullPath.c_str(), &entryStat) == 0 && S_ISDIR(entryStat.st_mode)) {
				// 递归删除子目录
				HBBDelFile(fullPath);
			}
			else {
				// 删除文件
				if (remove(fullPath.c_str()) != 0) {
					perror(("Failed to delete file: " + fullPath).c_str());
				}
			}
		}
		closedir(dir);

		// 删除空目录
		if (rmdir(path.c_str()) != 0) {
			perror(("Failed to delete directory: " + path).c_str());
		}
	}
	else {
		// 是文件，删除
		if (remove(path.c_str()) != 0) {
			perror(("Failed to delete file: " + path).c_str());
		}
	}
#endif
}

 long long GetDirSize(const char * path)
{
	if (!path)return 0;
	long long dir_size = 0;
	string dir_new = path;
	string name = "";
#ifdef _WIN32
	//存储文件信息
	_finddata_t file;
	dir_new += "\\*.*";

	intptr_t dir = _findfirst(dir_new.c_str(), &file);
	if (dir < 0)
		return 0;
	do
	{
		// 是否是文件夹，并且名称不为"."或".." 
		if (file.attrib & _A_SUBDIR)
		{
			name = file.name;
			if (name == "." || name == "..")
				continue;
			// 将dirNew设置为搜索到的目录，并进行下一轮搜索 
			dir_new = path;
			dir_new += "/";
			dir_new += name;
			dir_size += GetDirSize(dir_new.c_str());
		}
		else
		{
			dir_size += file.size;
		}
	} while (_findnext(dir, &file) == 0);
	_findclose(dir);
#else
	DIR *dp = 0;
	struct dirent *entry = 0;
	struct stat statbuf;
	dp = opendir(dir_new.c_str());
	if (dp == NULL)
		return 0;
	chdir(dir_new.c_str());
	char buf[1024] = { 0 };
	while ((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode))
		{
			name = entry->d_name;
			if (name == "." || name == "..")
				continue;
			dir_new = path;
			dir_new += "/";
			dir_new += entry->d_name;
			dir_size += GetDirSize(dir_new.c_str());
		}
		else
		{
			dir_size += statbuf.st_size;
		}
	}
	closedir(dp);
#endif
	return dir_size;
}


 bool GetDiskSize(const char *dir, unsigned long long *avail, unsigned long long *total, unsigned long long *free)
{
#ifdef _WIN32
	return GetDiskFreeSpaceExA(dir, (ULARGE_INTEGER *)avail, (ULARGE_INTEGER *)total, (ULARGE_INTEGER *)free);
#else
	struct statfs diskInfo;
	statfs(dir, &diskInfo);
	*total = diskInfo.f_blocks*diskInfo.f_bsize;
	*free = diskInfo.f_bfree*diskInfo.f_bsize;
	*avail = diskInfo.f_bavail*diskInfo.f_bsize;
	return true;
#endif
}