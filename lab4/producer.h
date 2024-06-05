#pragma once
#include "../queue/queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define PRODUCERS_MAX_COUNT 5

extern volatile sig_atomic_t runProducer;

void createProducer(void);
void deleteProducer(void);
void deleteAllProducers(void);
