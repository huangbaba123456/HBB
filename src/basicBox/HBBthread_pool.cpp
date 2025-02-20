
#include "HBBthread_pool.h"
#include "HBBthread.h"
#include <thread>
#include <iostream>
#include "HBBtask.h"
#include <thread>
#ifdef _WIN32
//��protobufͷ�ļ����г�ͻ ��protobuf��ͷ�ļ�Ҫ��windows.h֮ǰ
#include <windows.h>
#else
#include <signal.h>
#endif
using namespace std;

///�����̵߳�ѭ���˳��ж�
static bool is_exit_all = false;

///���е��̶߳���
static vector<HBBThread *>all_threads;
static mutex all_threads_mutex;
//////////////////////////////////////////////////////
///�˳����е��߳�
void HBBThreadPool::ExitAllThread()
{
    is_exit_all = true;
    all_threads_mutex.lock();
    for (auto t : all_threads)
    {
        t->Exit();
    }
    all_threads_mutex.unlock();
    this_thread::sleep_for(200ms);
}

void HBBThreadPool::Wait()
{
    while (!is_exit_all)
    {
        this_thread::sleep_for(100ms);
    }
}
class CHBBThreadPool :public HBBThreadPool
{
public:
    //�ַ��߳�
    void Dispatch(HBBTask *task)
    {
        //��ѯ
        if (!task)return;
        int tid = (last_thread_ + 1) % thread_count_;
        last_thread_ = tid;
        HBBThread *t = threads_[tid];

        t->AddTask(task);

        //�����߳�
        t->Activate();

    }
    //��ʼ�������̲߳������߳�
    void Init(int thread_count)
    {
        this->thread_count_ = thread_count;
        this->last_thread_ = -1;
        for (int i = 0; i < thread_count; i++)
        {
            HBBThread *t = new HBBThread();
            t->id = i + 1;
            //�����߳�
            t->Start();
            threads_.push_back(t);
            all_threads_mutex.lock();
            all_threads.push_back(t);
            all_threads_mutex.unlock();
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
private:

    ///�߳�����
    int thread_count_ = 0;

    ///��һ�ηַ��õ��̣߳�������ѯ
    int last_thread_ = -1;

    ///�̳��̶߳���
    std::vector<HBBThread *>threads_;
};

HBBThreadPool *HBBThreadPoolFactory::Create()
{
    //socket���ʼ��
    static mutex mux;
    static bool is_init = false;
    mux.lock();
    if (!is_init)
    {
#ifdef _WIN32 
        //��ʼ��socket��
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#else
        //ʹ�öϿ�����socket���ᷢ�����źţ���ɳ����˳�
        if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
            return NULL;
#endif
        is_init = true;
    }
    mux.unlock();
    return new CHBBThreadPool();
}
