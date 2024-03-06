#include "pch.h"
#include "IOCPServer.h"

void WorkerThreadMain(HANDLE iocpHandle)
{
	DWORD bytesTransfered = 0;
	Session* session = nullptr;
	OverlappedEx* overlappedEx = nullptr;

	while (true)
	{
		bool ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransfered,
			(ULONG_PTR*)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);

		if (ret == false || bytesTransfered == 0)
			continue;

		cout << "Recv Data Len = " << bytesTransfered << endl;
		cout << "Recv Data IOCP = " << session->recvBuffer << endl;

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUF_SIZE;

		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL); // WSARecv 함수 다시 등록
	}

}

int IOCPServer::Run()
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


	// IOCP
// 스레드들의 WSARecv, WSASend의 결과를 큐에 넣어 저장 -> worker 스레드를 두어 이벤트가 완료된 큐에 있는 데이터들을 처리해줌
	HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); // CP(Completion Port)라는 큐를 생성
	vector<Session*> sessionManager;
	vector<thread> threads;
	// Worker Thread 생성
	for (int i = 0; i < 5; i++)
	{
		threads.push_back(thread([iocpHandle]()
		{
			WorkerThreadMain(iocpHandle);
		}));
	}

	while (true)
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);

		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
			return 0;

		// Main Thread에서 accept까지 처리

		Session* session = new Session();
		session->socket = clientSocket;
		sessionManager.push_back(session);

		cout << "Client Connected" << endl;

		// 함수는 똑같지만 인자가 다름
		// clientSocket을 CP에 등록 - iocpHandle 큐에 clientSocket을 등록
		::CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, /*key*/(ULONG_PTR)session, 0); // key값으로 session의 주소값을 넣어줌

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUF_SIZE;

		OverlappedEx* overlappedEx = new OverlappedEx(); // 기본 overlapped에 type을 추가함
		overlappedEx->type = IO_TYPE::READ;
		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL); // 메인에서 WSARecv 한번 호출해줌

	}
}