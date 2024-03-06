#pragma once
#include "Server.h"
class SocketServer :
    public Server
{
    virtual int Run() override;
};

