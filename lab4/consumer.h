#pragma once
#include "../queue/queue.h"
#include "../message/message.h"
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_COUNT_OF_CONSUMERS 5

extern volatile sig_atomic_t runConsumer;

void createConsumer(void);
void deleteConsumer(void);
void deleteAllConsumers(void);
