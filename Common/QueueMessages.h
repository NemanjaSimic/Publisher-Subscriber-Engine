#pragma once
#include <ws2tcpip.h>
#include <stdio.h>

typedef struct QueueMessagesElement {
	QueueMessagesElement* next;
	QueueMessagesElement* prev;
	char* message;
}QueueMsgEl;

typedef struct QueueMessages {
	QueueMessagesElement* first;
	QueueMessagesElement* last;
}QueueMessages;

void EnqueueMessage(QueueMessages* queue, char* newMessage);
char* DequeueMessage(QueueMessages* queue);