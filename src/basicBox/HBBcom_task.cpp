#define EVENT__HAVE_OPENSSL
#include "HBBcom_task.h"
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <iostream>
#include <string.h>
#include "HBBtools.h"
#include "event2/bufferevent_ssl.h"
#include <chrono>
#include <thread>
#include "HBBssl.h"
#include "HBBlog_client.h"
using namespace std;


static void SReadCB(struct bufferevent *bev, void *ctx)
{
    auto task = (HBBComTask*)ctx;
    task->ReadCB();
}
static void SWriteCB(struct bufferevent *bev, void *ctx)
{
    auto task = (HBBComTask*)ctx;
    task->WriteCB();
}

void SAutoConnectTimerCB(evutil_socket_t s, short w, void *ctx)
{
    auto task = (HBBComTask*)ctx;
    task->AutoConnectTimerCB();
}
void STimerCB(evutil_socket_t s, short w, void *ctx)
{
    auto task = (HBBComTask*)ctx;
    task->TimerCB();
}
static void SEventCB(struct bufferevent *bev, short what,void *ctx)
{
    auto task = (HBBComTask*)ctx;
    task->EventCB(what);
}

HBBComTask::HBBComTask()
{
    mux_ = new mutex;
}
HBBComTask::~HBBComTask()
{
    delete mux_;
}
//void XComTask::InitSSL()
//{
//    XSSLCtx::InitSSL();
//}

//bool XComTask::SetSSL(SSLVer ver, const char* ca_file, const char* key_file, const char *vali_ca )
//{
//    XMutex mux(mux_);
//    auto ssl_ctx = new XSSLCtx();
//    bool re = ssl_ctx->Init(ver, ca_file, key_file,vali_ca);
//    if (!re)
//    {
//        LOGERROR("XComTask::SetSSL failed!");
//        ssl_ctx->Close();
//        delete ssl_ctx;
//        return re;
//    }
//    this->ssl_ctx_ = ssl_ctx;
//    return true;
//}

//////////////////////////////////////////////////////////////
/// �趨�Զ������Ķ�ʱ��
void HBBComTask::SetAutoConnectTimer(int ms)
{
    if (!base())
    {
        LOGERROR("SetAutoConnectTimer failed : base not set!");
        return;
    }
    if(auto_connect_timer_event_)
    {
        event_free(auto_connect_timer_event_);
        auto_connect_timer_event_ = 0;
    }

    auto_connect_timer_event_ = event_new(base(), -1, EV_PERSIST, SAutoConnectTimerCB, this);
    if (!auto_connect_timer_event_)
    {
        LOGERROR("SetAutoConnectTimer  failed :event_new faield!");
        return;
    }
    int sec = ms / 1000; //��
    int us = (ms % 1000) * 1000;//΢��
    timeval tv = { sec,us };
    event_add(auto_connect_timer_event_, &tv);
}
    
/////////////////////////////////////////
///�Զ�������ʱ���ص�����
void HBBComTask::AutoConnectTimerCB()
{
    cout<<"."<<flush;
     //����������ӣ���ȴ������û�У���ʼ����
    if (is_connected())
        return ;
    if (!is_connecting())
        Connect();
}

//////////////////////////////////////////////////////////////
///�趨��ʱ�� ֻ������һ����ʱ�� ��ʱ����TimerCB()�ص�
/// ��Init�����е���
///@para ms ��ʱ���õĺ���
void HBBComTask::SetTimer(int ms)
{
    if (!base())
    {
        LOGERROR("SetTimer failed : base not set!");
        return;
    }

    timer_event_ = event_new(base(), -1, EV_PERSIST, STimerCB, this);
    if (!timer_event_)
    {
        LOGERROR("set timer failed :event_new faield!");
        return;
    }
    int sec = ms / 1000; //��
    int us = (ms % 1000) * 1000;//΢��
    timeval tv = { sec,us };
    event_add(timer_event_, &tv);
}
void HBBComTask::set_local_ip(const char *ip)
{
    strncpy(this->local_ip_, ip, sizeof(local_ip_));
}

void HBBComTask::set_server_ip(const char* ip)
{
    strncpy(this->server_ip_, ip, sizeof(server_ip_));
}

int HBBComTask::Read(void *data, int datasize)
{
    if (!bev_)
    {
        LOGERROR("bev not set");
        return 0;
    }
    int re = bufferevent_read(bev_, data, datasize);
    return re;
}

///�������ж�ʱ��
void HBBComTask::ClearTimer()
{
    if(auto_connect_timer_event_)
        event_free(auto_connect_timer_event_);
    auto_connect_timer_event_ = 0;
    if(timer_event_)
        event_free(timer_event_);
    timer_event_ = 0;
}

void HBBComTask::Close()
{
    {
        HBBMutex mux(mux_);
        is_connected_ = false;
        is_connecting_ = false;

        if (bev_)
        {
            //��������� BEV_OPT_CLOSE_ON_FREE���ͷ�ssl  ��socket
            //��һ�����ͷ�����ռ�ÿռ䣬�����б��е��ͷŲ���ͨ��event_base_loop(base, EVLOOP_NONBLOCK);ȡ��
            bufferevent_free(bev_);
            bev_ = NULL;
        }

        if (msg_.data)
            delete msg_.data;
        memset(&msg_, 0, sizeof(msg_));
        //if (ssl_ctx_) �������Լ�����
        //{
        //    ssl_ctx_->Close();
        //    delete ssl_ctx_;
        //    ssl_ctx_ = 0;
        //}
    }
    //�������Ӷ���ռ䣬����Ͽ���������Ҫ��������
    if(auto_delete_)
    {
        //����ʱ��
        ClearTimer();
        delete this;
    }
        
}
bool  HBBComTask::Write(const void *data, int size)
{
    HBBMutex mux(mux_);
    if (!bev_  || !data || size <= 0)return false;
    int re = bufferevent_write(bev_, data, size);
    if (re != 0)return false;
    return true;
}
//bool XComTask::Write(const XMsg *msg)
//{
//    if (!bev_ || !msg || !msg->data || msg->size <= 0)return false;
//    //1 д����Ϣͷ
//    int re = bufferevent_write(bev_, msg, sizeof(XMsgHead));
//    if (re != 0)return false;
//    //2 д����Ϣ����
//    re = bufferevent_write(bev_, msg->data, msg->size);
//    if (re != 0)return false;
//    return true;
//}
//����д��ص�
void HBBComTask::BeginWrite()
{
    if (!bev_)return;
    bufferevent_trigger(bev_, EV_WRITE, 0);
}

void HBBComTask::EventCB(short what)
{
    cout << "SEventCB:" << what << endl;
    if (what & BEV_EVENT_CONNECTED)
    {
        cout << "BEV_EVENT_CONNECTED" << endl;
        stringstream ss;
        ss << "connnect server " << server_ip_ << ":" << server_port_ << " success!";
        LOGINFO(ss.str().c_str());
        //֪ͨ���ӳɹ�
        is_connected_ = true;
        is_connecting_ = false;
        auto ssl = bufferevent_openssl_get_ssl(bev_);
        if (ssl)
        {
            HBBSSL HBBssl;
            HBBssl.set_ssl(ssl);
            HBBssl.PrintCert();
            HBBssl.PrintCipher();
        }
        //��ȡ���ص�ַ
        int sock = bufferevent_getfd(bev_);
        /*if (sock > 0)
        {
            sockaddr_in sin;
            memset(&sin, 0, sizeof(sin));
            int len = sizeof(sin);
            getsockname(sock, (sockaddr*)&sin, &len);
            char buf[16] = { 0 };
            evutil_inet_ntop(AF_INET, &sin.sin_addr.s_addr, buf, sizeof(buf));
            cout << "client ip is " << buf << endl;
        }*/
        ConnectedCB();
    }

    ///�˳�Ҫ����������
    if (what & BEV_EVENT_ERROR )
    {
        auto ssl = bufferevent_openssl_get_ssl(bev_);
        if (ssl)
        {
            HBBSSL HBBssl;
            HBBssl.set_ssl(ssl);
            HBBssl.PrintCert();
        }
        cout << "BEV_EVENT_ERROR " << endl;
        int sock = bufferevent_getfd(bev_);
        int err = evutil_socket_geterror(sock);
        //err = evutil_socket_geterror(sock);
        LOGDEBUG(evutil_socket_error_to_string(err));
        Close();
    }
    if (what & BEV_EVENT_TIMEOUT)
    {
        cout << "BEV_EVENT_TIMEOUT" << endl;
        Close();
    }
    if (what & BEV_EVENT_EOF)
    {
        cout << "BEV_EVENT_EOF" << endl;
        Close();
    }
}
bool HBBComTask::Connect()
{
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(server_port_);
    evutil_inet_pton(AF_INET, server_ip_, &sin.sin_addr.s_addr);
    HBBMutex mux(mux_);
    is_connected_ = false;
    is_connecting_ = false;
    
    //��ʼ��bufferevent
    if(!bev_)InitBev(-1);
    if (!bev_)
    {
        LOGERROR("HBBComTask::Connect failed! bev is null!");
        return false;
    }
    int re = bufferevent_socket_connect(bev_, (sockaddr*)&sin, sizeof(sin));
    if (re != 0)
    {
        return false;
    }
    //��ʼ����
    is_connecting_ = true;
    return true;
}
bool HBBComTask::InitBev(int sock)
{
    //mux_->lock();
    ///��bufferevent �������� 
    /// ����bufferevent������ -1�Զ�����socket



    if (!ssl_ctx())
    {
        bev_ = bufferevent_socket_new(base(), sock, BEV_OPT_CLOSE_ON_FREE);
        if (!bev_)
        {
            //mux_->unlock();
            LOGERROR("bufferevent_socket_new failed!");
            return false;
        }
    }
    else// ����ͨ��
    {
        auto xssl = ssl_ctx()->NewHBBSSL(sock);
        //�ͻ���
        if (sock < 0)
        {

            bev_ = bufferevent_openssl_socket_new(base(), sock, xssl.ssl()
                , BUFFEREVENT_SSL_CONNECTING,
                BEV_OPT_CLOSE_ON_FREE// bufferevent_free��ͬʱ�ر�socket��ssl
            );

        }
        else
        {//�����
            bev_ = bufferevent_openssl_socket_new(base(), sock, xssl.ssl()
                , BUFFEREVENT_SSL_ACCEPTING,
                BEV_OPT_CLOSE_ON_FREE// bufferevent_free��ͬʱ�ر�socket��ssl
            );
        }
        if (!bev_)
        {
            //mux_->unlock();
            LOGERROR("bufferevent_openssl_socket_new failed!");
            return false;
        }
    }
	// ���ö���ʱʱ��
	if (read_timeout_ms_ > 0) {
		timeval read_tv = {read_timeout_ms_/1000,read_timeout_ms_%1000};
		bufferevent_set_timeouts(bev_,&read_tv,0);
	}
	if (timer_ms_ > 0) {
		SetTimer(timer_ms_); // ���ö�ʱ��������
	}
    //���ûص�����
    bufferevent_setcb(bev_, SReadCB, SWriteCB, SEventCB, this);
    bufferevent_enable(bev_, EV_READ | EV_WRITE);
    //mux_->unlock();
    return true;
}

//��ӵ������б����
bool HBBComTask::Init()
{
    //���ַ���˻��ǿͻ���
    int comsock = this->sock();
    if (comsock <= 0)
        comsock = -1;
	//if (server_ip_[0] != '\0')
	//{
	//	// �ͻ���
	//	auto ssl_ctx_ = new HBBSSLCtx();
	//	ssl_ctx_->InitClient("");
	//	set_ssl_ctx(ssl_ctx_);
	//}
	//else
	//{
	//	// �����
	//	string crt_path = "./server.crt";  // ������֤��
	//	string key_path = "./server.key";  // ������˽Կ
	//	string ca_path = "";       // CA ֤��
	//	auto ssl_ctx_ = new HBBSSLCtx();
	//	ssl_ctx_->InitServer(crt_path.c_str(), key_path.c_str(), ca_path.c_str());
	//	//ssl_ctx_->Init(SERVER_TLS, "server.crt", "server.key", "client.crt");
	//	set_ssl_ctx(ssl_ctx_);
	//}

    {
        HBBMutex mux(mux_);
        InitBev(comsock);
    }
    //timeval tv = { 10,0 };
    //bufferevent_set_timeouts(bev_, &tv, &tv);

    //���ӷ�����
    if (server_ip_[0] == '\0')
    {
        return true;
    }
    
    //�Ͽ������Զ�����
    if(auto_connect_){
        SetAutoConnectTimer(3000);
    }
    //�ͻ���
    return Connect();
}

//////////////////////////////////////////////////////
///�������ӣ�����Ͽ������ٴ�������֪�����ӳɹ������߳�ʱ
bool HBBComTask::AutoConnect(int timeout_sec)
{
    //����������ӣ���ȴ������û�У���ʼ����
    if (is_connected())
        return true;
    if (!is_connecting())
        Connect();
    return WaitConnected(timeout_sec);
}
//////////////////////////////////////////////////////////////
///�ȴ����ӳɹ�
///@para timeout_sec ���ȴ�ʱ��
bool  HBBComTask::WaitConnected(int timeout_sec)
{
    //10�������һ��
    int count = timeout_sec * 100;
    for (int i = 0; i < count; i++)
    {
        if (is_connected())
            return true;
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    return is_connected();
}
