#ifndef HBBCOM_TASK_H
#define HBBCOM_TASK_H
#include "HBBtask.h"
#include "HBBmsg.h"
#include <string>
#include "HBBssl_ctx.h"

class HBBSSLCtx;

class  HBBComTask : public HBBTask
{
public:
    HBBComTask();
    virtual ~HBBComTask();

    // 开始连接服务器，调用成员 server_ip_ server_port_
    // 考虑自动重连
    virtual bool Connect();
    ///初始化bufferevent，客户端建立连接

    ///添加到线程池任务列表调用，包括客户端和服务端
    virtual bool Init();

    ///清理资源，对象空间释放根据auto_delete_
    virtual void Close();

    int Read(void *data, int datasize);


    void set_server_ip(const char* ip);
    const char *server_ip() { return server_ip_; }

    void set_server_port(int port) { this->server_port_ = port; }
    int server_port() { return this->server_port_; }
    
    //本地IP用于获取配置项
    // 客户端在连接成功后设置
    void set_local_ip(const char *ip);
    const char *local_ip() { return local_ip_; };

    //static void InitSSL();
    //virtual bool SetSSL(SSLVer ver, const char* ca_file, const char* key_file, const char *vali_ca = 0);
    //virtual void set_

    //XSSLCtx * ssl_ctx() { return ssl_ctx_; }

    ///事件回调函数
    virtual void EventCB(short what);

    //激活写入回调
    virtual void BeginWrite();

    //发送消息
    //virtual bool Write(const XMsg *msg);
    virtual bool Write(const void *data, int size);

    //连接成功的消息回调，由业务类重载
    virtual void ConnectedCB() {};

    //当关闭消息接收时，数据将发送到此函数，由业务模块重载
    virtual void ReadCB(void *data, int size) {}

    //接收到消息的回调，由业务类重载 返回true正常，
    //返回false退出当前的消息处理,不处理下一条消息
    //virtual bool ReadCB(const XMsg *msg) = 0;

    ///写入数据回调函数
    virtual void WriteCB() {};

    ///读取数据回调函数
    virtual void ReadCB() = 0;

    void set_is_recv_msg(bool isrecv) { this->is_recv_msg_ = isrecv; }


    //////////////////////////////////////////////////////////////
    ///等待连接成功
    ///@para timeout_sec 最大等待时间
    bool WaitConnected(int timeout_sec);

    //////////////////////////////////////////////////////
    ///建立连接，如果断开，会再次重连，知道连接成功，或者超时
    bool AutoConnect(int timeout_sec);


    bool is_connecting() { return is_connecting_; }
    bool is_connected() { return is_connected_; }

    //连接断开是否自动清理对象 包含清理定时器
    void set_auto_delete(bool is) { auto_delete_ = is; }

    //是否自动重连 默认不自动 要在添加到线程池之前做
    //设置自动重连 对象就不能自动清理
    void set_auto_connect(bool is) 
    {
        auto_connect_ = is;
        if(is)//自动重连 对象就不能自动清理
            auto_delete_ = false;
    }
    bool auto_connect(){return  auto_connect_;}
    //////////////////////////////////////////////////////////////
    ///设定定时器 只能设置一个定时器 定时调用TimerCB()回调
    /// 在Init函数中调用
    ///@para ms 定时调用的毫秒
    virtual void SetTimer(int ms);
    /////////////////////////////////////////
    ///定时器回调函数
    virtual void TimerCB() {}
    //////////////////////////////////////////////////////////////
    /// 设定自动重连的定时器

    virtual void SetAutoConnectTimer(int ms);
    /////////////////////////////////////////
    ///自动重连定时器回调函数
    virtual void AutoConnectTimerCB() ;

	void set_read_timeout_ms(int read_timeout_ms_) {
		this->read_timeout_ms_ = read_timeout_ms_;
	}
	void set_timer_ms(int ms) { timer_ms_ = ms; }

    //void set_ssl(struct ssl_st *ssl) { this->ssl_ = ssl; }
    //struct ssl_st * ssl() { return ssl_; }
	///清理所有定时器
	virtual void ClearTimer();
	bool auto_delete() { return auto_delete_; };
protected:

    //读取缓存
    char read_buf_[4096] = { 0 };

    //本地IP用于获取配置项
    char local_ip_[16] = { 0 };
    
    //XSSL *ssl_ = 0;
	// struct ssl_st *ssl_ = 0;

private:

	//TimerCB 定时调用时间
	int timer_ms_ = 0;

	// 读取超时时间 ms
	int read_timeout_ms_ = 0;

    //是否自动重连
    bool auto_connect_ = false;

    //定时器清理 close时不清理
    struct event * auto_connect_timer_event_ = 0;

    //连接断开是否自动清理对象
    bool auto_delete_ = true;

    bool InitBev(int sock);
    /// 服务器IP
    char server_ip_[16] = {0};

    ///服务器端口
    int server_port_ = 0;

    struct bufferevent *bev_ = 0;

    //数据包缓存
	Msg msg_;

    //是否接收消息，接收的消息会调用 void ReadCB(const XMsg *msg)
    //不接收消息调用 void ReadCB(void *data, int size)
    bool is_recv_msg_ = true;

    //客户单的连接状态 
    //1 未处理  => 开始连接 （加入到线程池处理）
    //2 连接中 => 等待连接成功
    //3 已连接 => 做业务操作
    //4 连接后失败 => 根据连接间隔时间，开始连接
    bool is_connecting_ = true; //连接中
    bool is_connected_ = false;
    std::mutex * mux_ = 0;

    //定时器清理 close时不清理
    struct event * timer_event_ = 0;
};

#endif