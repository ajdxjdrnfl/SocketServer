#pragma comment(lib, "Ws2_32")
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <thread>

using namespace std;

fd_set writes;

int main()
{
	WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	// 1) ���� ����
	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET) return 0;

	// 2) �����ϰ��� �ϴ� ������ ���� (connect)
	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(7777);

	if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;

	// sendto
	while (true) 
	{
		FD_ZERO(&writes);

		FD_SET(clientSocket, &writes);

		int retVal = ::select(0, nullptr, &writes, nullptr, nullptr); // read_set�� ���. �������� NULL�� ó��

		if (retVal == SOCKET_ERROR) break;

		if (FD_ISSET(clientSocket, &writes))
		{
			char sendBuffer[50] = "Hello world!";
			int sendSize = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);

			// ��Ŷ�۽Ž� ����ó��
			if (sendSize == SOCKET_ERROR)
				return 0;
		}
		

		this_thread::sleep_for(1s);

	}
	
	::closesocket(clientSocket);
	::WSACleanup();
}

