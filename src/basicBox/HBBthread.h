
#ifndef HBBTHREAD_H
#define HBBTHREAD_H

#include <list>
#include <mutex>
class HBBTask;
class HBBThread
{
public:

	//启动线程
	void Start();

	//线程入口函数
	void Main();

	//安装线程，初始化event_base和管道监听事件用于激活
	bool Setup();

	//收到主线程发出的激活消息（线程池的分发）
	void Notify(int fd, short which);

	//线程激活
	void Activate();

	//添加处理的任务，一个线程同时可以处理多个任务，共用一个event_base
	void AddTask(HBBTask *t);
	HBBThread();
	~HBBThread();

	//线程编号
	int id = 0;

    //退出线程
    void Exit()
    {
        is_exit_ = true;
    }
private:
    bool is_exit_ = false;
	int notify_send_fd_ = 0;
	struct event_base *base_ = 0;

	//任务列表
	std::list<HBBTask*> tasks_;
	//线程安全 互斥
	std::mutex tasks_mutex_;

};

#endif
