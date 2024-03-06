#include "pch.h"
#include "NonBlockingServer.h"

int NonBlockingServer::Run()
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


	if (listenSocket == INVALID_SOCKET) return 0;

	u_long on = 1;

	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET) // ����ŷ �������� ����
		return 0;

	// ������ ���� + ���ε� + listen������ ����� �Ȱ���

	// accept - ����ŷ�̹Ƿ� ������ �����Ҽ��� �����Ҽ��� �����Ƿ� while������ ������ ������

	while (true)
	{
		int addrLen = sizeof(serverAddr);
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&serverAddr, &addrLen);
		if (clientSocket == SOCKET_ERROR)
		{
			// accept�� ����ŷ���� �۵�
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