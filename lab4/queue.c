#include "queue.h"
#include <stdlib.h>
#include <sys/mman.h>
#include "../functions/functions.h"


void initializeQueue(Queue *queue) {
    int sharedMemoryID = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (sharedMemoryID == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    queue = (Queue*)mmap(NULL, SHM_SIZE,
                                      PROT_READ | PROT_WRITE,MAP_SHARED,
                                      sharedMemoryID, 0);

    if (queue == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    queue->head = queue->tail = 0;
    queue->maxCapacity = QUEUE_CAPACITY;
    queue->currentSize = 0;
    queue->addedMessages = 0;
    queue->removedMessages = 0;
}

void addMessageToQueue(Queue* queue, Message *message) {

    if (queue->currentSize == queue->maxCapacity) {
        printf("Queue is full\n");
        return;
    }
    queue->addedMessages++;
    queue->currentSize++;
    queue->messages[queue->tail] = *message;
    queue->tail++;
    if(queue->tail == queue->maxCapacity) {
        queue->tail = 0;
    }
}

Message removeMessageFromQueue(Queue* queue) {
    Message msg;
    msg.hash = 0;
    msg.size = 0;
    msg.type = 0;
    for (int i = 0; i < 256; i++){
        msg.data[i] = 0;
    }
    if (queue->currentSize == 0) {
        printf("Queue is empty\n");
        return msg;
    }

    Message recievedMessage = queue->messages[queue->head];
    queue->messages[queue->head] = msg;
    queue->removedMessages++;
    queue->currentSize--;
    queue->head++;
    if(queue->head == queue->maxCapacity) {
        queue->head = 0;
    }
    return  recievedMessage;
}
