
#ifndef HBBTASK_H
#define HBBTASK_H

class HBBSSLCtx;
class  HBBTask
{
public:

	//初始化任务
	virtual bool Init() = 0;

    int thread_id() { return thread_id_; }
    void set_thread_id(int thread_id) {  thread_id_ = thread_id; }

    int sock() { return sock_; }
    void set_sock(int sock) { this->sock_ = sock; }

    struct event_base *base() { return base_; }
    void set_base(struct event_base *base) { this->base_ = base; }
    //设定ssl上下文，设定后通信使用ssl,此空间由用户自己清理
    HBBSSLCtx *ssl_ctx() { return ssl_ctx_; }
    void set_ssl_ctx(HBBSSLCtx *ssl_ctx) { this->ssl_ctx_ = ssl_ctx; };
    
    void set_task_name(const char *name) { char * cp = task_name_; while (*cp++ = *name++); }
    const char *task_name() { return task_name_; }
private:
    char task_name_[1024] = { 0 };
    struct event_base *base_ = 0;
    HBBSSLCtx *ssl_ctx_ = 0;
    int sock_ = 0;
    int thread_id_ = 0;

};
#endif
