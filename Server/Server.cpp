// Server.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include <string.h>
#include <locale.h>

#pragma comment(lib, "ws2_32.lib")

#define BUF_SIZE 10240
#define PORT 4869

#define STATUS_OK					0
#define STATUS_PROCESS_NOT_CREATED	1

void reverseShell(/*LPCWSTR host, int port*/)
{
	SECURITY_ATTRIBUTES   sa;
	HANDLE   hRead, hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		MessageBox(NULL, L"Error   On   CreatePipe()", NULL, 0);
		return;
	}
	STARTUPINFO   si;
	PROCESS_INFORMATION   pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	WCHAR cmd[] = L"cmd /c ping 127.0.0.1";
	if (!CreateProcess(NULL, cmd
		, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		DWORD   dw = GetLastError();
		MessageBox(NULL, L"Error   on   CreateProcess()", NULL, 0);
		return;
	}
	CloseHandle(hWrite);
	char   buffer[4096] = { 0 };
	DWORD   bytesRead;
	char result[4096] = { 0 };
	while (true)
	{
		if (ReadFile(hRead, buffer, 4095, &bytesRead, NULL) == NULL)
			break;
		printf_s("%s", buffer);
		//strcat_s(result, 4096, buffer);
	}
	//printf_s("%s", result);
}


int spawn_shell(PROCESS_INFORMATION *pi, HANDLE *out_read, HANDLE *in_write)
{
	SECURITY_ATTRIBUTES sattr;
	STARTUPINFO si;
	HANDLE in_read, out_write;

	memset(pi, 0, sizeof(PROCESS_INFORMATION));

	// create communication pipes  
	memset(&sattr, 0, sizeof(SECURITY_ATTRIBUTES));
	sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
	sattr.bInheritHandle = TRUE;
	sattr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(out_read, &out_write, &sattr, 0)) {
		return STATUS_PROCESS_NOT_CREATED;
	}
	if (!SetHandleInformation(*out_read, HANDLE_FLAG_INHERIT, 0)) {
		return STATUS_PROCESS_NOT_CREATED;
	}

	if (!CreatePipe(&in_read, in_write, &sattr, 0)) {
		return STATUS_PROCESS_NOT_CREATED;
	}
	if (!SetHandleInformation(*in_write, HANDLE_FLAG_INHERIT, 0)) {
		return STATUS_PROCESS_NOT_CREATED;
	}

	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.hStdError = out_write;
	si.hStdOutput = out_write;
	si.hStdInput = in_read;
	si.dwFlags |= STARTF_USESTDHANDLES;

	WCHAR cmd[] = L"cmd";
	if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, pi))
	{
		return STATUS_PROCESS_NOT_CREATED;
	}

	CloseHandle(out_write);
	CloseHandle(in_read);

	return STATUS_OK;
}


void createProcessTest()
{
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	WCHAR cmd[] = L"NOTEPAD";
	CreateProcess(NULL, cmd, NULL, NULL,
		FALSE, 0, NULL, NULL, &si, &pi);
}


void spawnShellTest()
{

	PROCESS_INFORMATION pi;

	HANDLE pipe_read, pipe_write;

	int status = spawn_shell(&pi, &pipe_read, &pipe_write);

	DWORD rs;
	char *in_buf, *out_buf;
	DWORD in_buf_size, out_buf_size;

	// allocate transfer buffers
	in_buf = (char *)malloc(BUF_SIZE);
	out_buf = (char *)malloc(BUF_SIZE);
	if (!in_buf || !out_buf) {
		printf("failed to allocate memory for transfer buffers\n");
		return -1;
	}
	memset(in_buf, 0x00, BUF_SIZE);
	memset(out_buf, 0x00, BUF_SIZE);

	// test 
	strcpy_s(in_buf, strlen("dir\r\n") + 1, "dir\r\n");
	in_buf_size = strlen(in_buf);

	// write
	if (status == STATUS_OK)
	{
		WriteFile(pipe_write, in_buf, in_buf_size, &rs, 0);
	}

	// read
	do {
		out_buf_size = 0;
		if (PeekNamedPipe(pipe_read, NULL, 0, NULL, &out_buf_size, NULL)) {
			if (out_buf_size > 0) {
				out_buf_size = 0;
				rs = ReadFile(pipe_read, out_buf, BUF_SIZE, &out_buf_size, NULL);
				if (!rs && GetLastError() != ERROR_IO_PENDING) {
					out_buf_size = sprintf_s(out_buf, BUF_SIZE, "Error: ReadFile failed with %i\n", GetLastError());
				}
				else
				{
					printf_s("%s", out_buf);
				}
			}
		}
		else {
			out_buf_size = sprintf_s(out_buf, BUF_SIZE, "Error: PeekNamedPipe failed with %i\n", GetLastError());
		}
		Sleep(100);
	} while (status == STATUS_OK);

	CloseHandle(pipe_read);
	CloseHandle(pipe_write);

}


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

	//reverseShell();

	//createProcessTest();

	//spawnShellTest

	return 0;
}

