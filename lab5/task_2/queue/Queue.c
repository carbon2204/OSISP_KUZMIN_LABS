#include "Queue.h"
#include <string.h>

int isQueueFull(Queue *queue) {
    return queue->currentSize == queue->maxCapacity;
}

void initializeQueue(Queue** queue) {
    (*queue)->messages = (Message **)calloc(QUEUE_CAPACITY, sizeof(Message *));
    for (int i = 0; i < QUEUE_CAPACITY; i++) {
        (*queue)->messages[i] = NULL;
    }

    (*queue)->tail = (*queue)->head = 0;
    (*queue)->maxCapacity = QUEUE_CAPACITY;
    (*queue)->currentSize = 0;
    (*queue)->addedMessages = 0;
    (*queue)->removedMessages = 0;
}

void addMessageToQueue(Queue *queue, Message *message) {
    queue->messages[queue->tail] = message;
    queue->tail = (queue->tail + 1) % queue->maxCapacity;
    queue->currentSize++;
    queue->addedMessages++;
}

void removeMessageFromQueue(Queue *queue) {
    printf(GREEN);
    printMessage(queue->messages[queue->head]);
    printf(WHITE);
    free(queue->messages[queue->head]);
    queue->messages[queue->head] = NULL;
    queue->head = (queue->head + 1) % queue->maxCapacity;
    queue->currentSize--;
    queue->removedMessages++;
}

void freeQueue(Queue *queue) {
    if(queue->messages != NULL) {
        for (int i = 0; i < queue->maxCapacity; i++) {
            if (queue->messages[i] != NULL) {
                free(queue->messages[i]);
            }
        }
        free(queue->messages);
    }
}

int increaseQueueSize(Queue *queue) {
    int newSize = queue->maxCapacity + 1;

    Message **newMessages = (Message **)malloc(newSize * sizeof(Message *));
    if (newMessages == NULL) {
        return 0;
    }
    if(queue->messages != NULL) {
        for (int i = 0; i < queue->currentSize; i++) {
            newMessages[i] =
                    queue->messages[(queue->head + i) % queue->maxCapacity];
        }
        free(queue->messages);
    }
    queue->messages = newMessages;
    queue->head = 0;
    queue->tail = queue->currentSize % newSize;
    queue->maxCapacity = newSize;
    printf("Queue size increased to %d\n", newSize);
    return 1;
}

int decreaseQueueSize(Queue *queue) {
    int newSize = queue->maxCapacity - 1;
    if (isQueueFull(queue)) {
        printf("Queue is full\n");
        return 0;
    }

    Message **newMessages = (Message **)malloc(newSize * sizeof(Message *));
    if (newMessages == NULL) {
        return 0;
    }

    for (int i = 0; i < newSize; i++) {
        newMessages[i] =
                queue->messages[(queue->head + i) % queue->maxCapacity];
    }

    free(queue->messages);
    queue->messages = newMessages;
    queue->head = 0;
    queue->tail = queue->currentSize % newSize;
    queue->maxCapacity = newSize;
    printf("Queue size decreased to %d\n", newSize);
    return 1;
}

void printQueueInfo(Queue* queue) {
    printf(YELLOW);
    printf("Queue status:\n");
    printf("Queue current size: %d\n", queue->currentSize);
    printf("Queue max capacity: %d\n", queue->maxCapacity);
    printf("Queue count removed messages: %d\n", queue->removedMessages);
    printf("Queue count added messages: %d\n", queue->addedMessages);
    printf(WHITE);
}
