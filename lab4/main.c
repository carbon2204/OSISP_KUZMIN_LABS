#define _POSIX_C_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <signal.h>
#include <fcntl.h>

typedef struct _message{
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    char data[259];
}Message;

typedef struct _Ring{
    Message messages[10];
    size_t  head, 
            tail,
            count,
            add,
            get;
}Ring;


int main(int argc, char** argv)
{
    //Initialize semaphores | 0644 - 6 - owner can write and read, 4 - group can read, 4 - other users can read
    sem_t *ProducerSem = sem_open("/semproducer", O_CREAT, 0644, 1),
          *ConsumerSem = sem_open("/semconsumer", O_CREAT, 0644, 1),
          *MonoSem = sem_open("/semmono", O_CREAT, 0644, 1);
          
    if (ProducerSem == SEM_FAILED || ConsumerSem == SEM_FAILED || MonoSem == SEM_FAILED) {
        perror("Semaphore create error");
        sem_close(ProducerSem);
        sem_close(ConsumerSem);
        sem_close(MonoSem);
        return 1;
    }

    //Initialize shared memory
    int fd;
    // O_CREAT - create, if shared memoty doesn't exist, O_RDWR - open for read and write
    // S_IRUSR - owner can read, S_IWUSR - owner can write
    if ((fd = shm_open("/ringmem", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
        perror("shm_open");

    if (ftruncate(fd, sizeof(Ring)) == -1)
        perror("Ftruncate");

    Ring* ring;
    // MAP_SHARED - the mapping is shared between several processes 
    if ((ring = mmap(NULL, sizeof(Ring), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) //void*)-1)
        perror("mmap");

    ring->count = 0;
    ring->add = 0;
    ring->get = 0;
    ring->head = ring->tail = 0;

    size_t prod_count = 0,
        consumer_count = 0;
    pid_t prod[50] = {(pid_t)0};
    pid_t consumer[50] = {(pid_t)0};

    char* child_argv[2];
    char input[3] = {'\0'};

    puts("-(p/c) -> kill producer/cosnumer\np -> create producer\nc -> create consumer\ns -> output stat\nq -> exit\n");
    while(1)
    {
        scanf("%s", input);

        switch (input[0])
        {
        case '-':
            if ((input[1] == 'p' || input[1] == 'P') && prod_count > 0)
                kill(prod[prod_count-- - 1], SIGUSR1);
            else if ((input[1] == 'c' || input[1] == 'C') && consumer_count > 0)
                kill(consumer[consumer_count-- - 1], SIGUSR1);
        break;
        
        case 'P':
        case 'p':
            prod[prod_count++] = fork();
            if (prod[prod_count - 1] == -1 )
               perror("Producer fork");

            else if (prod[prod_count - 1] == 0) {
                child_argv[0] = (char*)malloc(sizeof(char) * 9);
                child_argv[1] = (char*)0;
                sprintf(child_argv[0], "Prod_%ld", prod_count); 
                if (execve("./producer", child_argv, NULL) == -1) {
                    perror("Producer execve");
                    free(child_argv[0]);
                    return 1;
                }
            }
            break;
        
        case 'C':
        case 'c':
            consumer[consumer_count++] = fork();
            if(consumer[consumer_count - 1] == -1 )
               perror("Consumer fork");
        
            else if(consumer[consumer_count - 1] == 0) {
                child_argv[0] = (char*)malloc(sizeof(char) * 13);
                child_argv[1] = (char*)0;
                sprintf(child_argv[0], "Consumer_%ld", consumer_count); 

                if (execve("./consumer", child_argv, (char**)0) == -1) {
                    perror("Consumer execve");
                    free(child_argv[0]);
                    return 1;
                }
            }
            break;
        
        case 'S':
        case 's':
            sem_wait(MonoSem); //waitng sem > 0 and -1
            printf("---Stat---\nMax count: 10\nCount: %ld\nAdded: %ld\nGetted: %ld\nProducers count: %ld\nConsumers count: %ld\n", ring->count, ring->add, ring->get, prod_count, consumer_count);
            sem_post(MonoSem); //sem +1
        break;

        case 'Q':
        case 'q':
            while (kill(prod[prod_count-- - 1], SIGUSR1) == 0);
            while (kill(consumer[consumer_count-- - 1], SIGUSR1) == 0);
               
            munmap(ring, sizeof(Ring)); //canceling the display
            close(fd);
            shm_unlink("/ringmem"); //delete shared memory from file system

            if (sem_close(ProducerSem) == -1) {
                perror("Sem_close producerSem");
                return 1;
            }
            if (sem_close(ConsumerSem) == -1) {
                perror("Sem_close consumerSem");
                return 1;
            }
            if (sem_close(MonoSem) == -1) {
                perror("Sem_close monoSem");
                return 1;
            }
            
            //delete semaphore from system
            sem_unlink("/semmono");
            sem_unlink("/semproducer");
            sem_unlink("/semconsumer");
            
            return 0;
            break;

        default:
            break;
        }
    }

    return 0;
}