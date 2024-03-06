#pragma once

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <thread>


#pragma comment(lib, "Ws2_32")

const int BUF_SIZE = 1000;

using namespace std;


struct Session
{
	// 캐스팅을 위해 overlappeed을 가장 맨 앞으로 조정 
	WSAOVERLAPPED overlapped = {};
	// ***
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[BUF_SIZE] = {};
	int recvBytes = 0;

};

enum IO_TYPE
{
	READ,
	WRITE,
	ACCEPT,
	CONNECT,
};

struct OverlappedEx
{
	WSAOVERLAPPED overlapped = {};
	int type = 0;
	// TODO
};