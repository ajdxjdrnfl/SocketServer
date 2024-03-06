#pragma once
#include "Server.h"
class IOCPServer :
    public Server
{
public:
    virtual int Run() override;
};

