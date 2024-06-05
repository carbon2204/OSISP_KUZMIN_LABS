#pragma once
#include "../message/Message.h"

#define QUEUE_CAPACITY 15
#define GREEN "\033[0;32m"
#define CIAN "\033[0;36m"
#define YELLOW "\033[0;33m"
#define WHITE "\033[0m"
#define RED "\033[0;31m"

typedef struct {
    Message** messages;
    int head;
    int tail;
    int maxCapacity;
    int addedMessages;
    int removedMessages;
    int currentSize;
} Queue;


void addMessageToQueue(Queue* queue, Message* message);
void removeMessageFromQueue(Queue* queue);
void initializeQueue(Queue** queue);
void printQueueInfo(Queue* queue);
int decreaseQueueSize(Queue *queue);
int increaseQueueSize(Queue *queue);
void freeQueue(Queue *queue);
int isQueueFull(Queue* queue);
