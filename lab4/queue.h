#pragma once
#include "../message/message.h"

#define QUEUE_CAPACITY 15

typedef struct {
    int head;
    int tail;
    int maxCapacity;
    int addedMessages;
    int removedMessages;
    int currentSize;
    Message messages[QUEUE_CAPACITY];
} Queue;

void addMessageToQueue(Queue* queue, Message* message);
Message removeMessageFromQueue(Queue* queue);
void initializeQueue(Queue* queue);
