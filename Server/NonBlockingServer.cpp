#include "pch.h"
#include "NonBlockingServer.h"

int NonBlockingServer::Run()
{

	WSADATA wsaData; // 소켓에 대한 데이터 구조체
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // 운영체제에 소켓통신을 시작할 것을 알림
		return 0;

	// 1) 소켓 생성
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


	if (listenSocket == INVALID_SOCKET) return 0;

	u_long on = 1;

	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET) // 논블로킹 소켓으로 선언
		return 0;

	// 소켓을 생성 + 바인드 + listen까지는 방식이 똑같음

	// accept - 논블로킹이므로 연결이 성공할수도 실패할수도 있으므로 while문으로 루프를 돌아줌

	while (true)
	{
		int addrLen = sizeof(serverAddr);
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&serverAddr, &addrLen);
		if (clientSocket == SOCKET_ERROR)
		{
			// accept는 논블로킹으로 작동
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			break;
		}

		cout << "Client Conneted!" << endl;

		// RECV
		while (true)
		{
			char recvBuffer[100];
			int recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
			if (recvLen == SOCKET_ERROR)
			{
				if (::WSAGetLastError() == WSAEWOULDBLOCK)
				{
					continue;
				}

				break;
			}
			cout << "recv Data Len = " << recvLen << endl;
			cout << "recv Data  = " << recvBuffer << endl;

			::this_thread::sleep_for(1s);
		}
	}
}