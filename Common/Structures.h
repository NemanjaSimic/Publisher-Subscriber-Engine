#pragma once

#include <ws2tcpip.h>
#include <stdio.h>
#include "QueueMessages.h"
#include "QueueSubscribers.h"
#include "Enumerations.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016
#define ADDRESS "127.0.0.1"

int Send(SOCKET socket, char* msgToSend, int len);
int Recv(SOCKET socket, char* buffer, int len);
bool Select(SOCKET Socket,FlagSelect flag);
bool InitializeWindowsSockets();
SOCKET Connect();

typedef struct ThreadParametar
{
	SOCKET socket;
	int idThread;
}ThreadParametar;


typedef struct Mutex
{
	Mutex* next;
	Mutex* prev;
	HANDLE semaphore;
}Mutex;

typedef struct MutexHead
{
	Mutex* first;
	Mutex* last;
}MutexHead;

void EnqueueMutex(MutexHead** headMutex, Mutex* newM);

