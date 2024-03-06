#include "pch.h"
#include "SocketServer.h"

int SocketServer::Run()
{
	WSADATA wsaData; // ���Ͽ� ���� ������ ����ü
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // �ü���� ��������� ������ ���� �˸�
		return 0;

	// 1) ���� ����
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

	// 4) accept + recv,send
	while (true)
	{
		SOCKADDR_IN clientAddr;
		::memset(&clientAddr, 0, sizeof(clientAddr));
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen); // ������ ���������� ���ŷ

		// RECV
		while (true)
		{

			char recvBuffer[200];

			int recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
			if (recvLen == SOCKET_ERROR)
			{

				exit(0);
			}
			cout << "recv Data Len = " << recvLen << endl;

			::this_thread::sleep_for(3s);
		}
	}
}