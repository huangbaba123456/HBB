#ifndef HBBROUTERSERVER_H
#define HBBROUTERSERVER_H
#include "HBBservice.h"
class HBBRouterServer :public HBBService
{
public:
    HBBServiceHandle* CreateServiceHandle() override;
};

#endif
