#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "..\Common\Structures.h"

#pragma comment(lib, "Ws2_32.lib")


void ChangeTopic(Topic* t);
void SerializePackage(Topic* t, char* message, char* bufferToSend);
int Publish(Topic* topic, char* message);
void EnterMessage(char* message);


SOCKET connectSocket;
int __cdecl main(int argc, char **argv)
{
	connectSocket = Connect();
	Topic* myTopic = (Topic*)malloc(sizeof(Topic));
	char message[200];
	int option;

	while (true)
	{
		option = -1;
		do {
			printf("\n-------MENI-------");
			printf("\n--1.Publish");
			printf("\n--2.Close");
			printf("\nOption:");
			scanf("%d", &option);
			getchar();
		} while (option != 1 && option != 2 && option != 3);
		int Result;
		switch (option)
		{
		case 1:	
				Result = Publish(myTopic, message);
			break;
		case 2:
			return 0;
			break;
		default:
			break;
		}
	}

	return 0;
}

void ChangeTopic(Topic* t)
{
	int signal = -1;
	int type = -1;
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
			printf("\nOdredite svoj tip(0-fuse,1-breaker):");
			scanf("%d", &type);
		}
		else
		{
			printf("\nOdredite svoj tip(2-sec_A,3-sec_V):");
			scanf("%d", &type);
		}
	} while (type != 0 && type != 1 && type != 2 && type != 3);
	(*t).type = (Type)type;
	getchar();
	(*t).num = (UINT)23;
}

void SerializePackage(Topic * myTopic, char * message, char * bufferToSend)
{
	*((Flag*)bufferToSend) = publisher;
	*((Signal*)(bufferToSend + sizeof(Flag))) = myTopic->signal;
	*((Type*)(bufferToSend + sizeof(Flag) + sizeof(Signal))) = myTopic->type;
	*((UINT*)(bufferToSend + sizeof(Flag) + sizeof(Signal) + sizeof(Type))) = myTopic->num;

	for (UINT i = 0; i < strlen(message); i++)
	{
		*((char*)(bufferToSend + sizeof(Flag) + sizeof(Topic) + sizeof(char)*i)) = message[i];
	}
}

int Publish(Topic * topic, char * message)
{
	int iResult;
	ChangeTopic(topic);
	EnterMessage(message);

	int sizeOfMsg = sizeof(Flag) + sizeof(Topic) + sizeof(char) * strlen(message);
	int sizeOfBuffer = sizeOfMsg + sizeof(int);

	char* bufferToSend = (char*)malloc(sizeOfBuffer);
	*((int*)bufferToSend) = sizeOfMsg;

	SerializePackage(topic, message, bufferToSend + sizeof(int));

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);
	for (int i = 0; i < 50; i++)
	{
		iResult = Send(connectSocket, bufferToSend, sizeOfBuffer);
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			free(bufferToSend);
			return 1;
		}

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
		Sleep(500);
	}
	free(bufferToSend);
	return 0;
}

void EnterMessage(char * message)
{
	do {
		printf("Unesi poruku:");
		fgets(message, 200, stdin);
		fflush(stdin);
	} while (message[0] == '\n');
}