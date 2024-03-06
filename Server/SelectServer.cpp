#include "pch.h"
#include "SelectServer.h"

int SelectServer::Run()
{
	WSADATA wsaData; // 소켓에 대한 데이터 구조체

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


	fd_set reads;

	vector<Session> sessions;

	sessions.reserve(100);

	while (true)
	{
		//소켓 셋 초기화
		FD_ZERO(&reads);

		// reads 셋에 listenSocket 등록
		FD_SET(listenSocket, &reads);

		// 소켓 등록 +알파
		for (Session& s : sessions)
		{
			FD_SET(s.socket, &reads);
		}
		// [옵션] 마지막 timeout 인자 설정 가능
		int retVal = ::select(0, &reads, nullptr, nullptr, nullptr); // read_set만 사용. 나머지는 NULL로 처리

		if (retVal == SOCKET_ERROR) break;

		if (FD_ISSET(listenSocket, &reads))
		{
			SOCKADDR_IN clientAddr;
			int addrLen = sizeof(clientAddr);
			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);

			if (clientSocket != INVALID_SOCKET)
			{
				if (::WSAGetLastError() == WSAEWOULDBLOCK)
					continue;
				cout << "Client Connected!" << endl;
				sessions.push_back(Session{ clientSocket });
			}
		}

		// 나머지 소켓 체크
		for (Session& s : sessions)
		{
			if (FD_ISSET(s.socket, &reads))
			{
				int recvLen = ::recv(s.socket, s.recvBuffer, BUF_SIZE, 0);
				if (recvLen <= 0)
				{
					continue;
				}

				s.recvBytes = recvLen;
				cout << "RecvData = " << s.recvBuffer << endl;
				cout << "RecvData Len = " << s.recvBytes << endl;
			}
		}
		this_thread::sleep_for(1s);
	}
}