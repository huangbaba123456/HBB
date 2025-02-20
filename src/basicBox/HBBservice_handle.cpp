
#include "HBBservice_handle.h"

void HBBServiceHandle::set_client_ip(const char*ip)
{
    if (!ip)return;
    strncpy(client_ip_, ip, sizeof(client_ip_));
}

HBBServiceHandle::HBBServiceHandle()
{
}


HBBServiceHandle::~HBBServiceHandle()
{
}
