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
		::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL); // WSARecv �Լ� �ٽ� ���
	}

}

int IOCPServer::Run()
{
	WSADATA wsaData; // ���Ͽ� ���� ������ ����ü

	vector<Session> sessions;

	// select �� set ������
	fd_set reads;
	fd_set writes;

	sessions.reserve(100);

	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // �ü���� ��������� ������ ���� �˸�
		return 0;

	// 1) ���� ���� - accept ��
	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);

	// 2) �ּ�/��Ʈ ��ȣ ����(bind)
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
// ��������� WSARecv, WSASend�� ����� ť�� �־� ���� -> worker �����带 �ξ� �̺�Ʈ�� �Ϸ�� ť�� �ִ� �����͵��� ó������
	HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); // CP(Completion Port)��� ť�� ����
	vector<Session*> sessionManager;
	vector<thread> threads;
	// Worker Thread ����
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

		// Main Thread���� accept���� ó��

		Session* session = new Session();
		session->socket = clientSocket;
		sessionManager.push_back(session);

		cout << "Client Connected" << endl;

		// �Լ��� �Ȱ����� ���ڰ� �ٸ�
		// clientSocket�� CP�� ��� - iocpHandle ť�� clientSocket�� ���
		::CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, /*key*/(ULONG_PTR)session, 0); // key������ session�� �ּҰ��� �־���

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUF_SIZE;

		OverlappedEx* overlappedEx = new OverlappedEx(); // �⺻ overlapped�� type�� �߰���
		overlappedEx->type = IO_TYPE::READ;
		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL); // ���ο��� WSARecv �ѹ� ȣ������

	}
}