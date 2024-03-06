#pragma once
#include "Server.h"

class NonBlockingServer : public Server
{
	virtual int Run() override;
};

