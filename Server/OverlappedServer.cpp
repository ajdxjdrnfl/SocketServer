#include "pch.h"
#include "OverlappedServer.h"

void CALLBACK RecvCallback(DWORD error, DWORD recvLen, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	// TODO
	cout << "Data Recv Len Callback = " << recvLen << endl;

	// �����͸� �ޱ� ���� ĳ����
	Session* session = (Session*)(overlapped); // Ŀ���� ����ü Session���� ĳ���ð��� -> WSAOVERLAPPED�� �� ������ �Ѱ��־��� ������
	cout << session->recvBuffer << endl;
}

int OverlappedServer::Run()
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

		// Overlapped �ݹ� �Լ� ���
		cout << "Client Connected" << endl;
		Session session = Session{ clientSocket };
		while (true)
		{
			WSABUF wsaBuf; // �ּ� + length
			wsaBuf.buf = session.recvBuffer;
			wsaBuf.len = BUF_SIZE;

			DWORD recvLen = 0;
			DWORD flags = 0;
			if (::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, RecvCallback) == SOCKET_ERROR)
			{
				// Recv ���ڸ��� ��κ� SOCKET�� ���� ���ϹǷ� PENDING üũ
				if (::WSAGetLastError() == WSA_IO_PENDING)
				{
					// TODO -- �ݹ� �Լ� ���

					// ���÷� Alertable Wait�� ���
					// Alertable Wait -> �ڵ尡 sleep�ϱ� ������ �̺�Ʈ �߻��ϸ� ����� callback�Լ� ȣ�� / ������ APCť�� ��ϵǾ� ����
					::SleepEx(INFINITE, TRUE);
					//SleepEx�� ������(������� �Ϸ�Ǹ�) �ٷ� �ݹ��Լ��� �Ѿ�� ��
					//�ϳ��� ������(���ν�����)�� ���� �۵��ϰ� �ֱ� ������
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