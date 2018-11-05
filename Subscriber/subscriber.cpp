#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include<WinSock2.h>
#include "..\Common\Structures.h"

#pragma comment(lib, "Ws2_32.lib")


void SerializePackage(Topic* t, char* bufferToSend);
void PickTopic(Topic* t);
int Subscribe(Topic* t);
SOCKET Connect();
DWORD WINAPI ListenForPublish(LPVOID param);
void Menu();



SOCKET connectSocket;
CRITICAL_SECTION cs;
Topic* myTopic;

int main()
{
	connectSocket = Connect();
	InitializeCriticalSection(&cs);
	DWORD threadID;
	HANDLE thread = CreateThread(NULL, 0, &ListenForPublish, NULL, 0, &threadID);
	myTopic = (Topic*)malloc(sizeof(Topic));
	int option;
	while (true)
	{
		option = -1;
		do {
			printf("\n-------MENI-------");
			printf("\n--1.Subscribe");
			printf("\n--2.Close");
			printf("\nOption:");
			scanf("%d", &option);
			getchar();
		} while (option != 1 && option != 2);
		switch (option)
		{
		case 1:
			Subscribe(myTopic);
			break;
		case 2:
			return 0;
			break;
		default:
			break;
		}
	}
	DeleteCriticalSection(&cs);
	return 0;
}

void SerializePackage(Topic * myTopic, char * bufferToSend)
{
	*((Flag*)bufferToSend) = subscriber;
	*((Signal*)(bufferToSend + sizeof(Flag))) = myTopic->signal;
	*((Type*)(bufferToSend + sizeof(Flag) + sizeof(Signal))) = myTopic->type;
	*((UINT*)(bufferToSend + sizeof(Flag) + sizeof(Signal) + sizeof(Type))) = myTopic->num;
}

void PickTopic(Topic* t)
{
	int signal = -1;
	char type;
	printf("\nPodesite svoj topic:");
	do {
		printf("\nOdredite svoj signal(0-status,1-analog):");
		scanf("%d", &signal);
	} while (signal != 1 && signal != 0);
	(*t).signal = (Signal)signal;
	getchar();
	do {
		if (signal == 0)
		{
			printf("\nOdredite svoj tip(0-fuse,1-breaker,*-oba):");
			scanf("%c", &type);
		}
		else
		{
			printf("\nOdredite svoj tip(2-sec_A,3-sec_V,*-oba):");
			scanf("%c", &type);
		}
	} while (type != '0' && type != '1' && type != '2' && type != '3' && type != '*');
	if (type == '*')
	{
		(*t).type = all;
	}
	else
	{
		(*t).type = (Type)((int)type-48);
		getchar();
		(*t).num = 99;
	}
}

int Subscribe(Topic* t)
{
	int iResult;
	PickTopic(t);
	char* bufferToSend = (char*)malloc(sizeof(Flag) + sizeof(Topic) + sizeof(int));
	*((int*)bufferToSend) = sizeof(Topic) + sizeof(Flag);
	SerializePackage(t, bufferToSend + sizeof(int));

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);
	//do {

		iResult = Send(connectSocket, bufferToSend,sizeof(Flag) + sizeof(Topic) + sizeof(int));
		if (iResult == SOCKET_ERROR)
		{
			
			closesocket(connectSocket);
			WSACleanup();
			free(bufferToSend);
			return 1;
		}
	//} while (iResult > 0);

	free(bufferToSend);

	char* buffer = (char*)malloc(sizeof(int));
	do
	{
		iResult = Recv(connectSocket, buffer, sizeof(int));
		int sizeOfMsg = *((int*)buffer);

		char* bufferToRecv = (char*)malloc(sizeof(char) * sizeOfMsg);

		iResult = Recv(connectSocket, bufferToRecv, sizeOfMsg);
		if (iResult > 0)
		{
			int brojac = 0;
			while (bufferToRecv[brojac] != '\n')
			{
				brojac++;
			}
			char* poruka = (char*)malloc(sizeof(char)*(brojac + 1));
			for (int i = 0; i <= brojac; i++)
			{
				*(poruka + i) = *((char*)(bufferToRecv + sizeof(char)*i));
			}
			poruka[brojac + 1] = '\0';
			printf("\nMessage from engine: %s\n", poruka);
			free(bufferToRecv);
		}
	} while (iResult < 0);
	return 0;
}

DWORD WINAPI ListenForPublish(LPVOID param)
{
	char* buffer = (char*)malloc(sizeof(int));
	int iResult;

	while (true)
	{		
		iResult = Recv(connectSocket, buffer, sizeof(int));
		int sizeOfMsg = *((int*)buffer);
		
		char* bufferToRecv = (char*)malloc(sizeof(char) * sizeOfMsg);
		
		iResult = Recv(connectSocket, bufferToRecv, sizeOfMsg);
		if (iResult > 0)
		{
			int brojac = 0;
			while (bufferToRecv[brojac] != '\n')
			{				
				brojac++;
			}
			char* poruka = (char*)malloc(sizeof(char)*(brojac+1));
			for (int i = 0; i <= brojac; i++)
			{
				*(poruka+i) = *((char*)(bufferToRecv + sizeof(char)*i));
			}
			poruka[brojac + 1] = '\0';
			EnterCriticalSection(&cs);
			printf("\nPublish: %s\n", poruka);
			Menu();
			LeaveCriticalSection(&cs);
			free(bufferToRecv);
		}
		Sleep(700);
	}
}

void Menu()
{
			printf("\n-------MENI-------");
			printf("\n--1.Subscribe");
			printf("\n--2.Close");
			printf("\nOption:");
}