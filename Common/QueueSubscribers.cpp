#include "QueueSubscribers.h"

bool EnqueueSubscriber(QueueSubscribers* queue, Subscriber* subscriber)
{
	if (!SubscriberExsists(queue, subscriber))
	{
		QueueSubscribersElement* newElement = (QueueSubscribersElement*)malloc(sizeof(QueueSubscribersElement));
		newElement->subscriber = subscriber;

		if (queue->first == NULL)
		{
			queue->first = newElement;
			queue->last = newElement;
			newElement->prev = NULL;
			newElement->next = NULL;
			return true;
		}
		else
		{
			QueueSubscribersElement* temp = queue->first;
			queue->first = newElement;
			newElement->next = temp;
			temp->prev = newElement;
			newElement->prev = NULL;
			return true;
		}
	}
	return false;
}

Subscriber* DequeueSubscriber(QueueSubscribers* queue)
{

	if (queue->last == NULL)
	{
		return NULL;
	}
	else
	{
		Subscriber* retVal =  queue->last->subscriber;
		QueueSubscribersElement* temp = queue->last->prev;
		free(queue->last);
		queue->last = temp;
		if (temp != NULL)
		{
			temp->next = NULL;
		}
		return retVal;
	}
}

bool SubscriberExsists(QueueSubscribers* queue,Subscriber* subscriber)
{
	if (queue == NULL)
	{
		return false;
	}
	QueueSubscribersElement* temp = queue->first;
	while (temp != NULL)
	{
		if (subscriber->socket == temp->subscriber->socket)
		{
			return true;
		}
		temp = temp->next;
	}
	return false;
}

Switcher* TopicExists(Switcher* head, Topic * topic)
{
	if (head == NULL)
	{
		return NULL;
	}
	Switcher* temp = head;
	while (temp != NULL)
	{
		if (CheckTopics(temp->topic, *topic))
		{
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

bool CheckTopics(Topic  firstT, Topic  secondT)
{
	if (firstT.signal == secondT.signal && firstT.type == secondT.type)
	{
		return true;
	}
	return false;
}


Switcher* AddTopic(Switcher** head, Topic * topic)
{
	Switcher* newEl;
		newEl = TopicExists(*head, topic);
		if (newEl != NULL)
		{
			return newEl;
		}
		newEl = (Switcher*)malloc(sizeof(Switcher));
		newEl->topic = *topic;
		newEl->queueSubscribes = (QueueSubscribers*)malloc(sizeof(QueueSubscribers));
		newEl->queueSubscribes->first = NULL;
		newEl->queueSubscribes->last = NULL;

		if (*head == NULL)
		{
			*head = newEl;
			newEl->next = NULL;
		}
		else
		{
			Switcher* temp = *head;
			*head = newEl;
			newEl->next = temp;
		}
		return newEl;
}
