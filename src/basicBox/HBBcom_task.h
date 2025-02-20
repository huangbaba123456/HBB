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

    // ��ʼ���ӷ����������ó�Ա server_ip_ server_port_
    // �����Զ�����
    virtual bool Connect();
    ///��ʼ��bufferevent���ͻ��˽�������

    ///��ӵ��̳߳������б���ã������ͻ��˺ͷ����
    virtual bool Init();

    ///������Դ������ռ��ͷŸ���auto_delete_
    virtual void Close();

    int Read(void *data, int datasize);


    void set_server_ip(const char* ip);
    const char *server_ip() { return server_ip_; }

    void set_server_port(int port) { this->server_port_ = port; }
    int server_port() { return this->server_port_; }
    
    //����IP���ڻ�ȡ������
    // �ͻ��������ӳɹ�������
    void set_local_ip(const char *ip);
    const char *local_ip() { return local_ip_; };

    //static void InitSSL();
    //virtual bool SetSSL(SSLVer ver, const char* ca_file, const char* key_file, const char *vali_ca = 0);
    //virtual void set_

    //XSSLCtx * ssl_ctx() { return ssl_ctx_; }

    ///�¼��ص�����
    virtual void EventCB(short what);

    //����д��ص�
    virtual void BeginWrite();

    //������Ϣ
    //virtual bool Write(const XMsg *msg);
    virtual bool Write(const void *data, int size);

    //���ӳɹ�����Ϣ�ص�����ҵ��������
    virtual void ConnectedCB() {};

    //���ر���Ϣ����ʱ�����ݽ����͵��˺�������ҵ��ģ������
    virtual void ReadCB(void *data, int size) {}

    //���յ���Ϣ�Ļص�����ҵ�������� ����true������
    //����false�˳���ǰ����Ϣ����,��������һ����Ϣ
    //virtual bool ReadCB(const XMsg *msg) = 0;

    ///д�����ݻص�����
    virtual void WriteCB() {};

    ///��ȡ���ݻص�����
    virtual void ReadCB() = 0;

    void set_is_recv_msg(bool isrecv) { this->is_recv_msg_ = isrecv; }


    //////////////////////////////////////////////////////////////
    ///�ȴ����ӳɹ�
    ///@para timeout_sec ���ȴ�ʱ��
    bool WaitConnected(int timeout_sec);

    //////////////////////////////////////////////////////
    ///�������ӣ�����Ͽ������ٴ�������֪�����ӳɹ������߳�ʱ
    bool AutoConnect(int timeout_sec);


    bool is_connecting() { return is_connecting_; }
    bool is_connected() { return is_connected_; }

    //���ӶϿ��Ƿ��Զ�������� ��������ʱ��
    void set_auto_delete(bool is) { auto_delete_ = is; }

    //�Ƿ��Զ����� Ĭ�ϲ��Զ� Ҫ����ӵ��̳߳�֮ǰ��
    //�����Զ����� ����Ͳ����Զ�����
    void set_auto_connect(bool is) 
    {
        auto_connect_ = is;
        if(is)//�Զ����� ����Ͳ����Զ�����
            auto_delete_ = false;
    }
    bool auto_connect(){return  auto_connect_;}
    //////////////////////////////////////////////////////////////
    ///�趨��ʱ�� ֻ������һ����ʱ�� ��ʱ����TimerCB()�ص�
    /// ��Init�����е���
    ///@para ms ��ʱ���õĺ���
    virtual void SetTimer(int ms);
    /////////////////////////////////////////
    ///��ʱ���ص�����
    virtual void TimerCB() {}
    //////////////////////////////////////////////////////////////
    /// �趨�Զ������Ķ�ʱ��

    virtual void SetAutoConnectTimer(int ms);
    /////////////////////////////////////////
    ///�Զ�������ʱ���ص�����
    virtual void AutoConnectTimerCB() ;

	void set_read_timeout_ms(int read_timeout_ms_) {
		this->read_timeout_ms_ = read_timeout_ms_;
	}
	void set_timer_ms(int ms) { timer_ms_ = ms; }

    //void set_ssl(struct ssl_st *ssl) { this->ssl_ = ssl; }
    //struct ssl_st * ssl() { return ssl_; }
	///�������ж�ʱ��
	virtual void ClearTimer();
	bool auto_delete() { return auto_delete_; };
protected:

    //��ȡ����
    char read_buf_[4096] = { 0 };

    //����IP���ڻ�ȡ������
    char local_ip_[16] = { 0 };
    
    //XSSL *ssl_ = 0;
	// struct ssl_st *ssl_ = 0;

private:

	//TimerCB ��ʱ����ʱ��
	int timer_ms_ = 0;

	// ��ȡ��ʱʱ�� ms
	int read_timeout_ms_ = 0;

    //�Ƿ��Զ�����
    bool auto_connect_ = false;

    //��ʱ������ closeʱ������
    struct event * auto_connect_timer_event_ = 0;

    //���ӶϿ��Ƿ��Զ��������
    bool auto_delete_ = true;

    bool InitBev(int sock);
    /// ������IP
    char server_ip_[16] = {0};

    ///�������˿�
    int server_port_ = 0;

    struct bufferevent *bev_ = 0;

    //���ݰ�����
	Msg msg_;

    //�Ƿ������Ϣ�����յ���Ϣ����� void ReadCB(const XMsg *msg)
    //��������Ϣ���� void ReadCB(void *data, int size)
    bool is_recv_msg_ = true;

    //�ͻ���������״̬ 
    //1 δ����  => ��ʼ���� �����뵽�̳߳ش���
    //2 ������ => �ȴ����ӳɹ�
    //3 ������ => ��ҵ�����
    //4 ���Ӻ�ʧ�� => �������Ӽ��ʱ�䣬��ʼ����
    bool is_connecting_ = true; //������
    bool is_connected_ = false;
    std::mutex * mux_ = 0;

    //��ʱ������ closeʱ������
    struct event * timer_event_ = 0;
};

#endif