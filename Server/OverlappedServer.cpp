#include "pch.h"
#include "OverlappedServer.h"

void CALLBACK RecvCallback(DWORD error, DWORD recvLen, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	// TODO
	cout << "Data Recv Len Callback = " << recvLen << endl;

	// 데이터를 받기 위해 캐스팅
	Session* session = (Session*)(overlapped); // 커스텀 구조체 Session으로 캐스팅가능 -> WSAOVERLAPPED를 맨 앞으로 넘겨주었기 때문에
	cout << session->recvBuffer << endl;
}

int OverlappedServer::Run()
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

	while (true)
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);

		SOCKET clientSocket;
		while (true)
		{
			clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
			{
				break;
			}
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			return 0;
		}

		// Overlapped 콜백 함수 방식
		cout << "Client Connected" << endl;
		Session session = Session{ clientSocket };
		while (true)
		{
			WSABUF wsaBuf; // 주소 + length
			wsaBuf.buf = session.recvBuffer;
			wsaBuf.len = BUF_SIZE;

			DWORD recvLen = 0;
			DWORD flags = 0;
			if (::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, RecvCallback) == SOCKET_ERROR)
			{
				// Recv 하자마자 대부분 SOCKET을 받지 못하므로 PENDING 체크
				if (::WSAGetLastError() == WSA_IO_PENDING)
				{
					// TODO -- 콜백 함수 방식

					// 예시로 Alertable Wait를 사용
					// Alertable Wait -> 코드가 sleep하긴 하지만 이벤트 발생하면 깨어나서 callback함수 호출 / 스레드 APC큐에 등록되어 있음
					::SleepEx(INFINITE, TRUE);
					//SleepEx가 끝나면(입출력이 완료되면) 바로 콜백함수로 넘어가게 됨
					//하나의 스레드(메인스레드)만 현재 작동하고 있기 때문임
				}
				else
				{
					break;
				}
			}

			cout << "Data Recv = " << session.recvBuffer << endl;
		}
	}
}