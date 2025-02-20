
#ifndef HBBTHREAD_H
#define HBBTHREAD_H

#include <list>
#include <mutex>
class HBBTask;
class HBBThread
{
public:

	//�����߳�
	void Start();

	//�߳���ں���
	void Main();

	//��װ�̣߳���ʼ��event_base�͹ܵ������¼����ڼ���
	bool Setup();

	//�յ����̷߳����ļ�����Ϣ���̳߳صķַ���
	void Notify(int fd, short which);

	//�̼߳���
	void Activate();

	//��Ӵ��������һ���߳�ͬʱ���Դ��������񣬹���һ��event_base
	void AddTask(HBBTask *t);
	HBBThread();
	~HBBThread();

	//�̱߳��
	int id = 0;

    //�˳��߳�
    void Exit()
    {
        is_exit_ = true;
    }
private:
    bool is_exit_ = false;
	int notify_send_fd_ = 0;
	struct event_base *base_ = 0;

	//�����б�
	std::list<HBBTask*> tasks_;
	//�̰߳�ȫ ����
	std::mutex tasks_mutex_;

};

#endif
