// Server.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include <string.h>
#include <locale.h>

#pragma comment(lib, "ws2_32.lib")

#define BUF_SIZE 1024
#define PORT 4869

int wmain()
{
	setlocale(LC_CTYPE, "");
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	WSAStartup(sockVersion, &wsaData);

	SOCKET servSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	servAddr.sin_port = PORT;

	bind(servSocket, (SOCKADDR*)&servAddr, sizeof(servAddr));

	SOCKADDR_IN remoteAddr;
	int nAddrlen = sizeof(remoteAddr);

	while (true)
	{
		WCHAR recvData[BUF_SIZE];
		int r = recvfrom(servSocket, (char*)recvData, BUF_SIZE * 2, 0, (SOCKADDR*)&remoteAddr, &nAddrlen);
		if (r > 0)
		{
			wprintf_s(L"%ls\n", recvData);

			//WCHAR *sendData = L"收到";
			//sendto(servSocket, (char*)sendData, (wcslen(sendData) + 1) * 2, 0, (SOCKADDR*)&remoteAddr, nAddrlen);
		}
		else if (r == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		Sleep(10);
	}

	closesocket(servSocket);
	WSACleanup();

	return 0;
}

