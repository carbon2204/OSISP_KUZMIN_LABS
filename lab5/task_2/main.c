#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "queue/Queue.h"

#define MAX_PRODUCERS 5
#define MAX_CONSUMERS 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
Queue* queue;

int num_producers = 0;
int num_consumers = 0;
pthread_t producer_threads[MAX_PRODUCERS];
pthread_t consumer_threads[MAX_CONSUMERS];

void *producer(void *arg) {
    pthread_cleanup_push((void (*)(void *))pthread_mutex_unlock, (void *)&mutex);
    while (1) {
        pthread_mutex_lock(&mutex);

        while (queue->currentSize == queue->maxCapacity) {
            pthread_cond_wait(&empty, &mutex);
        }


        printf(YELLOW);
        printf("\nProducer thread with ID: %lu\n", pthread_self());
        printf(CIAN);
        Message* message = createMessage();
        addMessageToQueue(queue, message);
        printMessage(message);

        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&full);

        printf(WHITE);

        printf("\nCount added messages: %u\n", queue->addedMessages);
        sleep(5);
    }
    pthread_cleanup_pop(1);
}

void *consumer(void *arg) {
    pthread_cleanup_push((void (*)(void *))pthread_mutex_unlock, (void *)&mutex);
    while (1) {
        pthread_mutex_lock(&mutex);

        while (queue->currentSize == 0) {
            pthread_cond_wait(&full, &mutex);
        }

        printf(YELLOW);
        printf("\nConsumer thread with ID: %lu\n", pthread_self());
        printf(GREEN);
        removeMessageFromQueue(queue);

        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&empty);

        printf(WHITE);
        printf("\nCount removed messages: %d\n", queue->removedMessages);
        sleep(3);
    }
    pthread_cleanup_pop(1);
}

void create_producer() {
    if (num_producers < MAX_PRODUCERS)
    {
        pthread_create(&producer_threads[num_producers], NULL, producer, NULL);
        num_producers++;
    }
    else
    {
        printf("Cannot create more producers. Maximum limit reached.\n");
    }
}

void create_consumer() {
    if (num_consumers < MAX_CONSUMERS)
    {
        pthread_create(&consumer_threads[num_consumers], NULL, consumer, NULL);
        num_consumers++;
    }
    else
    {
        printf("Cannot create more consumers. Maximum limit reached.\n");
    }
}

void delete_producer()
{
    if (num_producers > 0)
    {
        pthread_cancel(producer_threads[num_producers - 1]);
        pthread_join(producer_threads[num_producers - 1], NULL);
        printf(RED);
        printf("\nProducer thread with ID %lu was deleted\n", producer_threads[num_producers - 1]);
        printf(WHITE);
        num_producers--;
    }
    else
    {
        printf(YELLOW);
        printf("No producers to delete.\n");
        printf(WHITE);
    }
}

void delete_consumer()
{
    if (num_consumers > 0)
    {
        pthread_cancel(consumer_threads[num_consumers - 1]);
        pthread_join(consumer_threads[num_consumers - 1], NULL);
        printf(RED);
        printf("\nConsumer thread with ID %lu was deleted\n", consumer_threads[num_consumers - 1]);
        printf(WHITE);
        num_consumers--;
    }
    else
    {
        printf(YELLOW);
        printf("There is no consumers to delete.\n");
        printf(WHITE);
    }
}

void end_program()
{
    for (int i = num_producers - 1; i >= 0; i--)
    {
        pthread_cancel(producer_threads[i]);
        pthread_join(producer_threads[i], NULL);
    }
    for (int i = num_consumers - 1; i >= 0; i--)
    {
        pthread_cancel(consumer_threads[i]);
        pthread_join(consumer_threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    freeQueue(queue);
    free(queue);
    exit(0);
}

int main()
{
    queue = (Queue*)malloc(sizeof(Queue));
    initializeQueue(&queue);
    int choice;

    while (1)
    {
        printf("\nMenu:\n");
        printf("1. Create producer\n");
        printf("2. Create consumer\n");
        printf("3. Delete last producer\n");
        printf("4. Delete last consumer\n");
        printf("5. Display queue statistics\n");
        printf("q. End program\n");
        printf("Enter your choice: ");
        choice = getchar();
        getchar();

        switch(choice)
        {
            case '1':
            {
                create_producer();
                break;
            }
            case '2':
            {
                create_consumer();
                break;
            }
            case '3':
            {
                delete_producer();
                break;
            }
            case '4':
            {
                delete_consumer();
                break;
            }
            case '5':
            {
                printQueueInfo(queue);
                break;
            }
            case 'q':
            {
                end_program();
                return 0;
            }
            case '+':
            {
                pthread_mutex_lock(&mutex);
                if (increaseQueueSize(queue))
                {
                    pthread_cond_signal(&empty);
                }
                pthread_mutex_unlock(&mutex);
                break;
            }
            case '-':
            {
                pthread_mutex_lock(&mutex);
                if (decreaseQueueSize(queue))
                {
                    pthread_cond_signal(&full);
                }
                pthread_mutex_unlock(&mutex);
                break;
            }
            default:
                printf("Invalid choice. Please, try again.\n");
        }
    }
}
