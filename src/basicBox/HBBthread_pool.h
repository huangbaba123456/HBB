#ifndef HBBTHREADPOOL_H_
#define HBBTHREADPOOL_H_

#include <vector>
class HBBThread;
class HBBTask;
class  HBBThreadPool
{
public:
    ///////////////////////////////////////////////////////////////////////////
    /// @brief ��ȡHBBThreadPool�ľ�̬���� ����̬������
    /// @return HBBThreadPool ��̬�����ָ��
    ///////////////////////////////////////////////////////////////////////////
	//static HBBThreadPool* Get()
	//{
	//	static HBBThreadPool p;
	//	return &p;
	//}
    ///////////////////////////////////////////////////////////////////////////
    /// @brief ��ʼ�������̲߳������̣߳�������event_base ,�����߳��п�ʼ������Ϣ
	virtual void Init(int thread_count) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// @brief �ַ������߳���ִ�У������task��Init���������ʼ��
    ///        �������ѯ�ַ����̳߳��еĸ����߳�
    /// @param task ����ӿڶ���HBBTask��Ҫ�û��Լ��̳в�����Init����
	virtual void Dispatch(HBBTask *task) = 0;
    
    //////////////////////////////////////////////////////
    ///�˳����е��߳�
    static void ExitAllThread();

    ///���� �ȴ�ExitAllThread
    static void Wait();

};

class  HBBThreadPoolFactory
{
public:
    //�����̳߳ض���
    static HBBThreadPool *Create();
};

#endif