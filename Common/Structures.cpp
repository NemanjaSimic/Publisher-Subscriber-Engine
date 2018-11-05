#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Structures.h"
#include <winsock2.h>

SOCKET Connect()
{
	SOCKET connectSocket = INVALID_SOCKET;

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(ADDRESS);
	serverAddress.sin_port = htons(DEFAULT_PORT);
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	return connectSocket;
}

int Send(SOCKET socket, char* msgToSend, int len)
{
	int count = 0;
	int checkError;
	while (count < len)
	{
		if (Select(socket,sendFlag))
		{
			checkError = send(socket, msgToSend + count, len - count, 0);
			if (checkError != SOCKET_ERROR)
			{
				count += checkError;
			}
		}
	}
	return count;
}

int Recv(SOCKET socket, char* buffer, int len)
{
	int count = 0;
	int checkError;
	while (count < len)
	{
		if (Select(socket,recvFlag))
		{
			checkError = recv(socket, buffer + count, len - count, 0);
			if (checkError != SOCKET_ERROR)
			{
				count += checkError;
			}
		}
	}
	return count;
}

bool Select(SOCKET Socket,FlagSelect flag)
{
	FD_SET set;
	timeval timeVal;
	int iResult;
	FD_ZERO(&set);
	FD_SET(Socket, &set);

	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;
	while (true)
	{
		if (flag == recvFlag)
		{
			iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		}
		else
		{
			iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);
		}

		if (iResult == SOCKET_ERROR)
		{
			return false;
		}
		else if (iResult == 0)
		{
			Sleep(500);
			continue;
		}
		else
		{
			return true;
		}
	}
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void EnqueueMutex(MutexHead** headMutex,Mutex* newM)
{
	if ((*headMutex)->first == NULL)
	{
		newM->next = NULL;
		(*headMutex)->first = newM;
		(*headMutex)->last = newM;
	}
	else
	{
		Mutex* temp = (*headMutex)->first;
		(*headMutex)->first = newM;
		newM->next = temp;
		temp->prev = newM;
	}
}
