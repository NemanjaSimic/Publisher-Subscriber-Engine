#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h> 
#include "..\Common\Structures.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORTS "27016"


int InitializeListenSocket(SOCKET* socket,const char* port);
DWORD WINAPI AcceptConnection(LPVOID listenSocket);
DWORD WINAPI ShutDown(LPVOID param);
void DeserializePakcage(Topic* t, char* bufferToRecv);
Subscriber* Subscribe(Topic * topic, SOCKET acceptedSocket,HANDLE semaphore);
void AcceptPublish(Topic* topic, char* message);
void AcceptMessage(char* message, Switcher* switcher);
DWORD WINAPI Publish(LPVOID param);
void NotifyOne();
void WaitForTable(HANDLE semaphore);
void MakeSubscriber(Topic *t, SOCKET acceptedSocket, int threadId);

Switcher* headSwitch = NULL;
MutexHead * headMutex;
CRITICAL_SECTION cs;
int IDniti = 0;

HANDLE niti[50];
DWORD nitiID[50];

int  main(void) 
{
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET acceptedSocket[50];

	//Inicijalizovanje mutexa(struktura koja ce otpustati semafore tredovima koji stanu u red za koriscenje resura)
	headMutex = (MutexHead*)malloc(sizeof(MutexHead));
	headMutex->first = NULL;
	headMutex->last = NULL;
	int kraj = 0;
	niti[IDniti] = CreateThread(NULL, 0, &ShutDown, &kraj, 0, &nitiID[IDniti]);
	IDniti++;
	InitializeCriticalSection(&cs);

	if(InitializeListenSocket(&listenSocket, DEFAULT_PORTS) == 1)
	{
		printf("\nERROR! Listen failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	
    do
    {
		//Listen socket je u neblokirajucem rezimu,select proverava da li je soket spreman za citanje
		if (Select(listenSocket,recvFlag) == false)
			continue;

        acceptedSocket[IDniti] = accept(listenSocket, NULL, NULL);

        if (acceptedSocket[IDniti] == INVALID_SOCKET)
        {        
			continue;
        }

		
		ThreadParametar* tp = (ThreadParametar*)malloc(sizeof(ThreadParametar));
		tp->socket = acceptedSocket[IDniti];
		tp->idThread = IDniti;
		//Svaka konekcija dobija svoju nit
		niti[IDniti] = CreateThread(NULL, 0, &AcceptConnection, tp, 0, &nitiID[IDniti]);
		IDniti++;
    } while (kraj == 0);

	for (int i = 0; i < IDniti; i++)
	{
		CloseHandle(niti[i]);
		closesocket(acceptedSocket[i]);
		WSACleanup();
	}

	free(headMutex);


	
	Switcher* tempS = headSwitch;
	while (tempS != NULL)
	{
		if (tempS->queueSubscribes->first != NULL)
		{
            QueueSubscribersElement* temp = headSwitch->queueSubscribes->first;
			QueueSubscribersElement* temp1 = temp->next;
			while (1)
			{
				free(temp->subscriber);
				free(temp);
				if (temp1 != NULL)
				{
					QueueSubscribersElement* pom = temp1;
					temp1 = temp1->next;
					temp = pom;
				}
				else
				{
					break;
				}
			}
		}
		//free((void*)(tempS->topic));
		tempS = tempS->next;
	}
	free(headSwitch);
    closesocket(listenSocket);
    WSACleanup();
	DeleteCriticalSection(&cs);
    return 0;
}

int InitializeListenSocket(SOCKET* listenSocket, const char* port)
{
	unsigned long int nonBlockingMode = 1;
	int iResult;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// Prepare address information structures
	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

									 // Resolve the server address and port
	iResult = getaddrinfo(NULL,(PCSTR)port, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	*listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (*listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(*listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(*listenSocket);
		WSACleanup();
		return 1;
	}

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);

	iResult = ioctlsocket(*listenSocket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	// Set listenSocket in listening mode
	iResult = listen(*listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(*listenSocket);
		WSACleanup();
		return 1;
	}
	printf("Server initialized, waiting for clients.\n");
	return 0;
}

DWORD WINAPI AcceptConnection(LPVOID param)
{
	ThreadParametar* tp = (ThreadParametar*)param;
	SOCKET acceptedSocket = tp->socket;
	int threadId = tp->idThread;
	free(tp);
	int iResult;
	unsigned long int nonBlockingMode = 1;
	while (true)
	{
		iResult = ioctlsocket(acceptedSocket, FIONBIO, &nonBlockingMode);

		if (iResult == SOCKET_ERROR)
		{
			printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
			return 1;
		}

		
			char* buffer = (char*)malloc(sizeof(int));
			iResult = Recv(acceptedSocket, buffer, sizeof(int));
			int sizeOfMsg = *((int*)buffer);
			free(buffer);

			char* bufferToRecv = (char*)malloc(sizeof(char) * sizeOfMsg);

			iResult = Recv(acceptedSocket, bufferToRecv, sizeOfMsg);

			if (iResult > 0)
			{		
				Flag flag = *((Flag*)bufferToRecv);
				Topic* t = (Topic*)malloc(sizeof(Topic));
				DeserializePakcage(t, bufferToRecv);
				
				char* topicChar = "";
				switch (t->signal)
				{
				case status:
					if (t->type == fuse)
					{
						topicChar = "Signal: status; Type: fuse.";
					}
					else if (t->type == breaker)
					{
						topicChar = "Signal: status; Type: breaker.";
					}
					else
					{
						topicChar = "Signal: status; Type: *.";
					}
					break;
				case analog:
					if (t->type == sec_A)
					{
						topicChar = "Signal: analog; Type: sec_A.";
					}
					else if (t->type == sec_V)
					{
						topicChar = "Signal: analog; Type: sec_V.";
					}
					else
					{
						topicChar = "Signal: analog; Type: *.";
					}
					break;
				default:
					break;
				}

				if (flag == subscriber)
				{
					bool threadForPublish = false;
					char* msg = "Already subscribed!\n";
					if (t->type == all)
					{
						if (t->signal == analog)
						{
							t->type = sec_A;
							MakeSubscriber(t, acceptedSocket, threadId);
							t->type = sec_V;
							MakeSubscriber(t, acceptedSocket, threadId);
						}
						else
						{
							t->type = fuse;
							MakeSubscriber(t, acceptedSocket, threadId);
							t->type = breaker;
							MakeSubscriber(t, acceptedSocket, threadId);
						}
					}
					else
					{
						MakeSubscriber(t, acceptedSocket, threadId);
					}
					printf("\nSubscriber at socket %d, subscribed on topic %s", acceptedSocket, topicChar);

					free(t);
					free(bufferToRecv);

					continue;

				}
				else if (flag == publisher)
				{
					char* message = (char*)malloc(sizeOfMsg - sizeof(Flag) - sizeof(Topic));
					for (UINT i = 0; i < sizeOfMsg - sizeof(Topic); i++)
					{
						*(message + i) = *((char*)(bufferToRecv + sizeof(Flag) + sizeof(Topic) + sizeof(char)*i));
					}

					HANDLE semaphoreForTable = CreateSemaphore(0, 0, 1, NULL);
					WaitForTable(semaphoreForTable);
					WaitForSingleObject(semaphoreForTable, INFINITE);
					AcceptPublish(t, message);
					NotifyOne();
					CloseHandle(semaphoreForTable);
					printf("\nPublisher at socket %d published topic: %s",acceptedSocket,topicChar);
					free(t);
					//free(message);
					char* msg = "Successfully published!\n";
					char* bufferT = (char*)malloc(sizeof(int) + sizeof(char)*strlen(msg));
					*((int*)bufferT) = strlen(msg);
					for (UINT i = 0; i < strlen(msg); i++)
					{
						*((char*)(bufferT + sizeof(int) + sizeof(char)*i)) = msg[i];
					}
					Send(acceptedSocket, bufferT, sizeof(int) + sizeof(char)*strlen(msg));
					free(bufferT);
					free(bufferToRecv);
					continue;
				}
				else
				{
					printf("\nERROR! Bad format of package!");
					free(bufferToRecv);
				}			

				/*iResult = shutdown(acceptedSocket, SD_SEND);
				if (iResult == SOCKET_ERROR)
				{
					printf("shutdown failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocket);
					WSACleanup();
					free(bufferToRecv);					
					return 1;
				}		*/	
			}
			else
			{
				continue;
			}
		}
	return 0;
}

void DeserializePakcage(Topic* t,char* bufferToRecv) 
{
	t->signal = *((Signal*)(bufferToRecv + sizeof(Flag)));
	t->type = *((Type*)(bufferToRecv + sizeof(Flag) + sizeof(Signal)));
	t->num = *((UINT*)(bufferToRecv + sizeof(Flag) + sizeof(Signal) + sizeof(Type)));
}

Subscriber* Subscribe(Topic * topic,SOCKET acceptedSocket,HANDLE semaphore)
{
	Subscriber* newSubscriber = (Subscriber*)malloc(sizeof(Subscriber));
	newSubscriber->queueMessages = (QueueMessages*)malloc(sizeof(QueueMessages));
	newSubscriber->queueMessages->first = NULL;
	newSubscriber->queueMessages->last = NULL;
	newSubscriber->socket = acceptedSocket;
	newSubscriber->semaphore = semaphore;
	Switcher* sw = NULL;
	Topic* t = topic;
	sw = AddTopic(&headSwitch, t);
	if (!EnqueueSubscriber(sw->queueSubscribes, newSubscriber))
	{
			//vec se sub na ovaj topic
		free(topic);
		return NULL;
	}
	return newSubscriber;
}

void AcceptPublish(Topic* topic, char* message) 
{
	Switcher* sw = TopicExists(headSwitch, topic);
	if (sw != NULL)
	{
		AcceptMessage(message, sw);
	} 
}

void AcceptMessage(char* message, Switcher* switcher)
{
	QueueSubscribersElement* temp = switcher->queueSubscribes->first;
	while (temp != NULL)
	{
		EnqueueMessage(temp->subscriber->queueMessages,message);
		ReleaseSemaphore(temp->subscriber->semaphore, 1, NULL);
		temp = temp->next;
	}
}

DWORD WINAPI Publish(LPVOID param)
{
	ParametarForPublish* pfp = (ParametarForPublish*)param;
	SOCKET socket = pfp->socket;
	Subscriber* subscriber = pfp->subscriber;
	HANDLE semaphore = pfp->Semaphore;
	HANDLE semaphoreForTable = CreateSemaphore(0, 0, 1, NULL);
	free(pfp);
	int threadId = pfp->threadId;
	char* message;
	while (true)
	{
		WaitForSingleObject(semaphore, INFINITE);
		WaitForTable(semaphoreForTable);
		WaitForSingleObject(semaphoreForTable, INFINITE);
		message = DequeueMessage(subscriber->queueMessages);
		NotifyOne();	

		if (message != NULL)
		{
			char* bufferT = (char*)malloc(sizeof(int) + sizeof(char)*strlen(message));
			*((int*)bufferT) = strlen(message);
			for (UINT i = 0; i < strlen(message); i++)
			{
				*((char*)(bufferT + sizeof(int) + sizeof(char)*i)) = message[i];
			}
			Send(socket, bufferT, sizeof(int) + sizeof(char)*strlen(message));
			free(bufferT);
			free(message);
		}
	}
	return 0;
}

void NotifyOne()
{
	HANDLE tempSemaphore = NULL;
	EnterCriticalSection(&cs);
	if (headMutex->last != NULL)
	{
		Mutex* temp = headMutex->last->prev;
		if (temp != NULL)
		{
			tempSemaphore = temp->semaphore;
			temp->next = NULL;
		}
		else
		{
			headMutex->first = NULL;
		}
		free(headMutex->last);
		headMutex->last = temp;

	}
	LeaveCriticalSection(&cs);
	if (tempSemaphore != NULL)
	{
		ReleaseSemaphore(tempSemaphore, 1, NULL);
	}
}

void WaitForTable(HANDLE semaphore)
{
	Mutex* newM = (Mutex*)malloc(sizeof(Mutex));
	newM->semaphore = semaphore;
	newM->prev = NULL;

	EnterCriticalSection(&cs);
	if (headMutex->first == NULL)
	{
		newM->next = NULL;
		headMutex->first = newM;
		headMutex->last = newM;
		LeaveCriticalSection(&cs);
		ReleaseSemaphore(semaphore, 1, NULL);
	}
	else
	{
		Mutex* temp = headMutex->first;
		headMutex->first = newM;
		newM->next = temp;
		temp->prev = newM;
		LeaveCriticalSection(&cs);

	}
}
void MakeSubscriber(Topic *t, SOCKET acceptedSocket,int threadId)
{
	char* msg = "Already subscribed!\n";
	HANDLE SemaphoreForPublish = CreateSemaphore(0, 0, 1, NULL);
	bool threadForPublish = false;
	HANDLE semaphorForTable = CreateSemaphore(0, 0, 1, NULL);
	WaitForTable(semaphorForTable);
	WaitForSingleObject(semaphorForTable, INFINITE);
	Subscriber* subscriber = Subscribe(t, acceptedSocket, SemaphoreForPublish);
	if (subscriber != NULL)
	{
		threadForPublish = true;
		msg = "Successfully subscribed!\n";
	}
	NotifyOne();
	CloseHandle(semaphorForTable);

	char* bufferT = (char*)malloc(sizeof(int) + sizeof(char)*strlen(msg));
	*((int*)bufferT) = strlen(msg);
	for (UINT i = 0; i < strlen(msg); i++)
	{
		*((char*)(bufferT + sizeof(int) + sizeof(char)*i)) = msg[i];
	}
	Send(acceptedSocket, bufferT, sizeof(int) + sizeof(char)*strlen(msg));
	free(bufferT);

	if (threadForPublish)
	{
		ParametarForPublish* pfp = (ParametarForPublish*)malloc(sizeof(ParametarForPublish));
		pfp->socket = acceptedSocket;
		pfp->subscriber = subscriber;
		pfp->Semaphore = SemaphoreForPublish;
		EnterCriticalSection(&cs);

		pfp->threadId = IDniti;
		niti[IDniti] = CreateThread(NULL, 0, &Publish, pfp, 0, &nitiID[IDniti]);
		IDniti++;
		//CloseHandle(nitiID);
		LeaveCriticalSection(&cs);
		threadForPublish = true;
	}
}
DWORD WINAPI ShutDown(LPVOID param)
{
	int* kraj = (int*)param;
	while (true)
	{
		getchar();
		*kraj = 1;
		return 0;
	}
}
