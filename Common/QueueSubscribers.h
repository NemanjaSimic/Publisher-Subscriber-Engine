#pragma once
#include <ws2tcpip.h>
#include <stdio.h>
#include "Structures.h"
#include "QueueMessages.h"
#include "Enumerations.h"

typedef struct Topic
{
	Signal signal;
	Type type;
	UINT num;
}Topic;

typedef struct Subscriber
{
	QueueMessages* queueMessages;
	SOCKET socket;
	HANDLE semaphore;
}Subscriber;

typedef struct QueueSubscribersElement {
	QueueSubscribersElement* next;
	QueueSubscribersElement* prev;
	Subscriber* subscriber;
}QueueSubscribersElement;

typedef struct QueueSubscribers {
	QueueSubscribersElement* first;
	QueueSubscribersElement* last;
}QueueSubscribers;

typedef struct Switcher {
	Topic topic;
	QueueSubscribers* queueSubscribes;
	Switcher* next;
}Switcher;

typedef struct ParametarForPublish
{
	SOCKET socket;
	Subscriber* subscriber;
	int threadId;
	HANDLE Semaphore;
}ParametarForPublish;


bool EnqueueSubscriber(QueueSubscribers* queue, Subscriber* subscriber);
Subscriber* DequeueSubscriber(QueueSubscribers* queue);
Switcher* TopicExists(Switcher* head, Topic * topic);
bool CheckTopics(Topic  firstT, Topic  secondT);
Switcher* AddTopic(Switcher** head, Topic * topic);
bool SubscriberExsists(QueueSubscribers* queue, Subscriber* subscriber);
