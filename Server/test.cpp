#pragma comment(lib, "Ws2_32")
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <thread>

const int BUF_SIZE = 1000;

struct Session // 커스텀 구조체 선언
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[BUF_SIZE] = {};
	int recvBytes = 0;
};




using namespace std;


int WsaEventmain()
{
	WSADATA wsaData; // 소켓에 대한 데이터 구조체

	vector<Session> sessions;

	// select 용 set 데이터
	fd_set reads;
	fd_set writes;

	sessions.reserve(100);

	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // 운영체제에 소켓통신을 시작할 것을 알림
		return 0;

	// 1) 소켓 생성 - accept 용
	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);

	// 2) 주소/포트 번호 설정(bind)
	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;

	// 3) listen
	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;

	vector<WSAEVENT> wsaEvents;

	WSAEVENT listenEvent = ::WSACreateEvent();
	wsaEvents.push_back(listenEvent);
	sessions.push_back(Session{ listenSocket });

	// 이벤트와 소켓 등록
	if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR) // ACCEPT, CLOSE 관심있는 이벤트 등록
	{
		return 0;
	}

	while (true)
	{
		// 이벤트 대기
		int index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE); // 하나의 이벤트라도 완료되면 index 리턴
		if (index == WSA_WAIT_FAILED)
			continue;

		index -= WSA_WAIT_EVENT_0; // index에 시작 위치 반환

		// 이벤트 조사 : WSAEnumNetworkEvents
		WSANETWORKEVENTS networkEvents;
		if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
			continue;


		if (networkEvents.lNetworkEvents & FD_ACCEPT) // accept 이벤트 일 시
		{
			//Error-Check
			if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				continue;

			SOCKADDR_IN clientAddr;
			int addrLen = sizeof(clientAddr);

			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
			{
				cout << " Client Connected" << endl;
				WSAEVENT clientEvent = ::WSACreateEvent();
				wsaEvents.push_back(clientEvent);
				sessions.push_back(Session{ clientSocket });
				if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) return 0;
			}
		}

		// Client Session 소켓 체크
		if (networkEvents.lNetworkEvents & FD_READ)
		{
			if (networkEvents.iErrorCode[FD_READ_BIT] != 0)
				continue;
			Session& s = sessions[index];
			int recvLen = ::recv(s.socket, s.recvBuffer, BUF_SIZE, 0);
			if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
			{
				if (recvLen <= 0) continue;
			}
			s.recvBytes = recvLen;
			cout << "RecvData = " << s.recvBuffer << endl;
			cout << "RecvData Len = " << s.recvBytes << endl;
		}
	}

}

