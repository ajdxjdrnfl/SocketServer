#pragma once
#include "Server.h"

class OverlappedServer : public Server
{
public:
	virtual int Run() override;
};