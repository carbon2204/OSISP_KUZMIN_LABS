#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <unistd.h>

typedef struct _message {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    char data[259];
} Message;

typedef struct _Ring {
    Message messages[10];
    size_t head,
        tail,
        count,
        add,
        get;
} Ring;

bool flag = true;

void sig1_handler(int signo) {
    if (signo != SIGUSR1)
        return;

    flag = false;
}

int main(int argc, char** argv) {
    signal(SIGUSR1, sig1_handler);

    sem_t* ConsumerSem = sem_open("/semconsumer", 0);
    if (ConsumerSem == SEM_FAILED) {
        perror("Consumer sem_open");
        return 1;
    };
    sem_t* MonoSem = sem_open("/semmono", 0);
    if (ConsumerSem == SEM_FAILED) {
        perror("Consumer sem_open");
        return 1;
    };

    int fileDescriptor;
    Ring* ring;

    if ((fileDescriptor = shm_open("/ringmem", O_RDWR, S_IRUSR | S_IWUSR)) == -1)
        perror("shm_open");
    if ((ring = mmap(NULL, sizeof(Ring), PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0)) == (void*)-1)
        perror("Mmap");

    while (flag) {
        sem_wait(ConsumerSem);
        sem_wait(MonoSem);

        if (ring->count > 0) {
            printf("%s get message: %s\n\n", argv[0], ring->messages[ring->tail].data);

            ring->tail = ring->tail == 9 ? 0 : ring->tail + 1;

            ring->get++;
            ring->count--;
        }

        if (sem_post(MonoSem) != 0)
            perror("Producer semaphore unlock error");
        if (sem_post(ConsumerSem) != 0)
            perror("Producer semaphore unlock error");

        sleep(2);
    }

    munmap(ring, sizeof(Ring));

    if (sem_close(ConsumerSem) == -1)
        perror("Consumer semaphore close");
    if (sem_close(MonoSem) == -1)
        perror("Consumer semaphore close");
    return 0;
}