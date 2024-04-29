#define _POSIX_C_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/random.h>
#include <time.h>

typedef struct _message {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    char data[259];
}Message;

typedef struct _Ring {
    Message messages[10];
    size_t  head, 
            tail,
            count,
            add,
            get;
}Ring;

//FNV-1 - algorithm hashing
uint16_t hash_16(const void *data, size_t len) {
    const uint8_t *bytes = data;
    uint16_t hash = 0xFFFF;
    const uint16_t fnv_prime = 0x0101;

    for (size_t i = 0; i < len; ++i) {
        hash = hash * fnv_prime;
        hash = hash ^ bytes[i];
    }

    return hash;
}

Message generateMessage() {

    srand((unsigned)time(NULL));
    const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; 
    
    Message message;
    {
        uint16_t random;
        do {
            if (getrandom(&random, sizeof(random), 0) < 0){
                perror("getrandom");
                exit(1);
            }
            message.size = random = random % 257;
            if(random != 0)
                break;

        }while(1);
        
        for(int i = 0; i < random; message.data[i] = letters[rand() % 53], i++);
    }

    message.hash = hash_16(message.data, message.size);
    message.type = rand() % 256;

    return message;
}

bool flag = true;

void sig1_handler(int signo) {
    if(signo != SIGUSR1)
        return;
        
    flag = false;
}

int main(int argc, char** argv) {
    signal(SIGUSR1, sig1_handler);

    //Getting semaphore
    sem_t* ProducerSem = sem_open("/semproducer", 0);
    if (ProducerSem == SEM_FAILED) {
        perror("Producer sem_open");
        return 1;
    }

    sem_t* MonoSem = sem_open("/semmono", 0);

    if (MonoSem == SEM_FAILED) {
        perror("Producer sem_open");
        return 1;
    }

    int fileDescriptor;
    Ring* ring;

    //Getting shared mem struct
    if ((fileDescriptor = shm_open("/ringmem", O_RDWR, S_IRUSR | S_IWUSR)) == -1)
        perror("shm_open");
    if ((ring = mmap(NULL, sizeof(Ring), PROT_READ | PROT_WRITE, MAP_SHARED , fileDescriptor, 0)) == MAP_FAILED)//(void*)-1)
        perror("Mmap");

    while (flag) {
        
        sem_wait(ProducerSem);
        sem_wait(MonoSem);

        //Generating message
        if (ring->count < 10){
            ring->messages[ring->head] = generateMessage();

            printf("%s generate message: %s\n\n", argv[0], ring->messages[ring->head].data);

            ring->head = ring->head == 9 ? 0 : ring->head + 1;

            ring->add++;
            ring->count++;
        }


        if (sem_post(MonoSem) != 0)
            perror("Producer semaphore unlock error");
        if (sem_post(ProducerSem) != 0)
            perror("Producer semaphore unlock error");
       
        sleep(2);
        
    }
    

    if (munmap(ring, sizeof(Ring)) == -1)
        perror("Munmap");
    
    if (sem_close(ProducerSem) == -1)
        perror("Producer semaphore close");
    if (sem_close(MonoSem) == -1)
        perror("Producer semaphore close");
    
    return 0;
}   