
#include "HBBthread.h"
#include <thread>
#include <iostream>
#include <event2/event.h>
#include "HBBtask.h"
#ifdef _WIN32
#else
#include <unistd.h>
#endif

using namespace std;

//�����߳�����Ļص�����
static void NotifyCB(int fd, short which, void *arg)
{
	HBBThread *t = (HBBThread *)arg;
	t->Notify(fd, which);
}
void HBBThread::Notify(int fd, short which)
{
	//ˮƽ���� ֻҪû�н�����ɣ����ٴν���
	char buf[2] = { 0 };
#ifdef _WIN32
	int re = recv(fd, buf, 1, 0);
#else
	//linux���ǹܵ���������recv
	int re = read(fd, buf, 1);
#endif
	if (re <= 0)
		return;
	HBBTask *task = NULL;
	//��ȡ���񣬲���ʼ������
	tasks_mutex_.lock();
	if (tasks_.empty())
	{
		tasks_mutex_.unlock();
		return;
	}
	task = tasks_.front(); //�Ƚ��ȳ�
	tasks_.pop_front();
	tasks_mutex_.unlock();
	task->Init();
}

void HBBThread::AddTask(HBBTask *t)
{
	if (!t)return;
	t->set_base(this->base_);
	tasks_mutex_.lock();
	tasks_.push_back(t);
	tasks_mutex_.unlock();
}
//�̼߳���
void HBBThread::Activate()
{
#ifdef _WIN32
	int re = send(this->notify_send_fd_, "c", 1, 0);
#else
	int re = write(this->notify_send_fd_, "c", 1);
#endif
	if (re <= 0)
	{
		cerr << "HBBThread::Activate() failed!" << endl;
	}
}
//�����߳�
void HBBThread::Start()
{
	Setup();
	//�����߳�
	thread th(&HBBThread::Main,this);

	//�Ͽ������߳���ϵ
	th.detach();
}
//��װ�̣߳���ʼ��event_base�͹ܵ������¼����ڼ���
bool HBBThread::Setup()
{
	//windows�����socket linux�ùܵ�
#ifdef _WIN32
	//����һ��socketpair ���Ի���ͨ�� fds[0] �� fds[1]д 
	evutil_socket_t fds[2];
	if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0)
	{
		cout << "evutil_socketpair failed!" << endl;
		return false;
	}
	//���óɷ�����
	evutil_make_socket_nonblocking(fds[0]);
	evutil_make_socket_nonblocking(fds[1]);
#else
	//�����Ĺܵ� ������send recv��ȡ read write
	int fds[2];
	if (pipe(fds))
	{
		cerr << "pipe failed!" << endl;
		return false;
	}
#endif

	//��ȡ�󶨵�event�¼��У�д��Ҫ����
	notify_send_fd_ = fds[1];

	//����libevent�����ģ�������
	event_config *ev_conf = event_config_new();
	event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
	this->base_ = event_base_new_with_config(ev_conf);
	event_config_free(ev_conf);
	if (!base_)
	{
		cerr << "event_base_new_with_config failed in thread!" << endl;
		return false;
	}

	//��ӹܵ������¼������ڼ����߳�ִ������
	event *ev = event_new(base_, fds[0], EV_READ | EV_PERSIST, NotifyCB, this);
	event_add(ev, 0);

	return true;
}
//�߳���ں���
void HBBThread::Main()
{
    if (!base_)
    {
        cerr << "HBBThread::Main faield! base_ is null " << endl;
        cerr << "In windows set WSAStartup(MAKEWORD(2, 2), &wsa)" << endl;
        return;
    }

    //����Ϊ�������ַ���Ϣ
    while (!is_exit_)
    {
        //һ�δ��������Ϣ
        event_base_loop(base_, EVLOOP_NONBLOCK);
        this_thread::sleep_for(1ms);
    }
    

	//event_base_dispatch(base_);

	event_base_free(base_);

}


HBBThread::HBBThread()
{
}


HBBThread::~HBBThread()
{
}
