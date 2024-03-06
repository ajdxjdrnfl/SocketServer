#pragma once
#include "Server.h"

class WSAEventServer : public Server
{
public:
	virtual int Run() override;
};