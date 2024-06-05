#pragma once

#include "../consumer/consumer.h"
#include "../producer/producer.h"
#include "../queue/queue.h"
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <semaphore.h>

#define SHM_SIZE sizeof(Queue)  //(sizeof(Queue) + QUEUE_CAPACITY * (sizeof(Message) + 256))
#define SHARED_MEMORY_NAME "/sharedMemory"
#define SEM_FREE_SPACE "/semFreeSpace"
#define SEM_FILLED_SPACE "/semUsedSpace"
#define MUTEX "/mutex"
#define GREEN "\033[0;32m"
#define CIAN "\033[0;36m"
#define YELLOW "\033[0;33m"
#define WHITE "\033[0m"

void initializeHandler(void);
void menu(int sharedMemoryID,
          Queue* queue);
void initializeSharedMemory(int *sharedMemoryId,
                            Queue **queue,
                            char setSizeFlag);
void cleanResources(void);

void initializeAllSemaphores(sem_t **freeSpaceSemaphore,
                             sem_t **filledSpaceSemaphore,
                             sem_t **mutex);

void closeAllSemaphores(sem_t *freeSpaceSemaphore,
                        sem_t *filledSpaceSemaphore,
                        sem_t *mutex);

void openSemaphore(const char name[],
                   sem_t** semaphore,
                   int value);
