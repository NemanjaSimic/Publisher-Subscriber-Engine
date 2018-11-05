// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Common\Structures.h"



int main()
{

	//QueueMessages* head = (QueueMessages*)malloc(sizeof(QueueMessages));
	//head->first = NULL;
	//head->last = NULL;
	///*QueueMessagesElement* kju[10];
	//for (int i = 0; i < 10; i++)
	//{
	//	kju[i] = (QueueMessagesElement*)malloc(sizeof(QueueMessagesElement));
	//}*/
	//Sleep(2000);
	//for (int i = 0; i < 10; i++)
	//{
	//	//kju[i]->message = "test";
	//	EnqueueMessage(head, "test");
	//}
	//Sleep(2000);
	//for (int i = 0; i < 10; i++)
	//{
	//	DequeueMessage(head);
	//}
	//
	//free(head);



	QueueSubscribers* head = (QueueSubscribers*)malloc(sizeof(QueueSubscribers));
	head->first = NULL;
	head->last = NULL;
	Subscriber* niz[10];
	for (int i = 0; i < 10; i++)
	{
		niz[i] = (Subscriber*)malloc(sizeof(Subscriber));
		niz[i]->socket = i;
		niz[i]->semaphore = NULL;
		niz[i]->queueMessages = NULL;
	}
	Sleep(2000);
	for (int i = 0; i < 10; i++)
	{
		EnqueueSubscriber(head, niz[i]);
	}
	Sleep(2000);
	for (int i = 0; i < 10; i++)
	{
		DequeueSubscriber(head);
	}
	Sleep(2000);
	for (int i = 0; i < 10; i++)
	{
		free(niz[i]);
	}
	free(head);

    return 0;
}

