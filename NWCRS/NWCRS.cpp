// NWCRS.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string.h>
#include <locale.h>

#pragma comment(lib, "ws2_32.lib")

#define BUF_SIZE 1024
#define PORT 4869

int wmain()
{
	//����Ϊ���ر���
	setlocale(LC_CTYPE, "");
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	WSAStartup(sockVersion, &wsaData);

	SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	SOCKADDR_IN clientAddr;
	clientAddr.sin_family = AF_INET;
	IN_ADDR addr;
	InetPtonW(AF_INET, L"127.0.0.1", &addr);
	clientAddr.sin_addr.S_un.S_addr = addr.S_un.S_addr;
	clientAddr.sin_port = PORT;

	while (true)
	{
		WCHAR *sendData = L"���";
		sendto(clientSocket, (char*)sendData, (wcslen(sendData) + 1) * 2, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
		//wprintf_s(L"Client is send a msg\n");
		wprintf_s(L"�ж�˹̩\n");
		Sleep(1000);
	}

	closesocket(clientSocket);
	WSACleanup();

	return 0;
}

