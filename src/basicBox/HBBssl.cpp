
#include "HBBssl.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
using namespace std;

//�ͷ�SSL
void HBBSSL::Close()
{
    if (ssl_)
    {
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
        ssl_ = 0;
    }
}

int  HBBSSL::Write(const void *data, int data_size)
{
    if (!ssl_)return 0;
    return SSL_write(ssl_, data, data_size);
}

int  HBBSSL::Read(void *buf, int buf_size)
{
    if (!ssl_)return 0;
    return SSL_read(ssl_,buf, buf_size);
}
//��ӡ�Է�֤����Ϣ
void HBBSSL::PrintCert()
{
    if (!ssl_) return;
    //��ȡ��֤��
    auto cert = SSL_get_peer_certificate(ssl_);
    if (cert == NULL)
    {
        cout << "no certificate" << endl;
        return;
    }
    char buf[1024] = { 0 };
    auto sname = X509_get_subject_name(cert);
    auto str = X509_NAME_oneline(sname, buf, sizeof(buf));
    if (str)
    {
        cout << "subject:"<<str << endl;
    }
    //����
    auto issuer = X509_get_issuer_name(cert);
    str = X509_NAME_oneline(issuer, buf, sizeof(buf));
    if (str)
    {
        cout << "issuer:" << str << endl;
    }
    X509_free(cert);
}

//��ӡͨ��ʹ�õ��㷨
void HBBSSL::PrintCipher()
{
    if (!ssl_)return ;
    cout << SSL_get_cipher(ssl_) << endl;
}

//�ͻ��˴���ssl����
bool HBBSSL::Connect()
{
    //socket ��connect�Ѿ��������
    if (!ssl_)
        return false;
    int re = SSL_connect(ssl_);
    if (re <= 0)
    {
        cout << "HBBSSL::Connect() failed!" << endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    cout << "SSL_connect success!" << endl;
    PrintCipher();
    PrintCert();
    return true;
}
//����˽���ssl����
bool HBBSSL::Accept()
{
    if (!ssl_)
        return false;
    //����ssl������֤����ԿЭ��
    int re = SSL_accept(ssl_);
    if (re <= 0)
    {
        cout << "HBBSSL::Accept() failed!" << endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    cout << "SSL_accept success!" << endl;
    PrintCipher();
    return true;
}

HBBSSL::HBBSSL()
{
}


HBBSSL::~HBBSSL()
{
}
