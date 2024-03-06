#include "pch.h"
#include "SelectServer.h"

int SelectServer::Run()
{
	WSADATA wsaData; // ���Ͽ� ���� ������ ����ü

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


	fd_set reads;

	vector<Session> sessions;

	sessions.reserve(100);

	while (true)
	{
		//���� �� �ʱ�ȭ
		FD_ZERO(&reads);

		// reads �¿� listenSocket ���
		FD_SET(listenSocket, &reads);

		// ���� ��� +����
		for (Session& s : sessions)
		{
			FD_SET(s.socket, &reads);
		}
		// [�ɼ�] ������ timeout ���� ���� ����
		int retVal = ::select(0, &reads, nullptr, nullptr, nullptr); // read_set�� ���. �������� NULL�� ó��

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

		// ������ ���� üũ
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