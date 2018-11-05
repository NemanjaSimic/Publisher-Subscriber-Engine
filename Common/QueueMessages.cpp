#include "QueueMessages.h"



void EnqueueMessage(QueueMessages* queue, char* newMessage)
{

	QueueMessagesElement* newElement = (QueueMessagesElement*)malloc(sizeof(QueueMessagesElement));
	//newElement->message = (char*)malloc(sizeof(strlen(newMessage)));
	newElement->message = NULL;
	newElement->message = newMessage;
	if (queue->first == NULL)
	{
		queue->first = newElement;
		queue->last = newElement;
		newElement->next = NULL;
		newElement->prev = NULL;
	}
	else
	{
		QueueMessagesElement* temp = queue->first;
		queue->first = newElement;
		newElement->next = temp;
		temp->prev = newElement;
		newElement->prev = NULL;
	}
}

char* DequeueMessage(QueueMessages* queue)
{
	if (queue->last == NULL)
	{
		return NULL;
	}
	else
	{
		char* message = queue->last->message;
		QueueMessagesElement* temp = queue->last->prev;
		if (queue->last->prev == NULL)
		{
			free(queue->last);
			queue->first = NULL;
			queue->last = NULL;
		}
		else
		{
			temp->next = NULL;
			free(queue->last);
			queue->last = temp;
		}
		return message;
	}
}
