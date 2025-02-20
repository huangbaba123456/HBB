
#ifndef HBBSERVICE_HANDLE_H
#define HBBSERVICE_HANDLE_H
#include "HBBmsg_event.h"

class  HBBServiceHandle:public HBBMsgEvent
{
public:
    HBBServiceHandle();
    ~HBBServiceHandle();
    void set_client_ip(const char*ip);
    const char *client_ip() { return client_ip_; }

    void set_client_port(int port) { this->client_port_ = port; }
private:
    char client_ip_[16] = { 0 };
    int client_port_ = 0;
};


#endif