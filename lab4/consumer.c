#include "consumer.h"
#include "../functions/functions.h"
#include <sys/mman.h>

// Массив и количество PID дочерних процессов-потребителей
pid_t consumersPids[MAX_COUNT_OF_CONSUMERS];
size_t consumersCount = 0;
volatile sig_atomic_t runConsumer = 1;

/**
 * @brief Создает нового потребителя (процесс-потомок).
 *
 * Функция создает новый процесс-потребитель с помощью fork().
 * Дочерний процесс занимается обработкой сообщений из общей очереди.
 */
void createConsumer(void) {
    if (consumersCount == MAX_COUNT_OF_CONSUMERS) {
        printf("Max count of consumers is reached\n");
        return;
    }

    pid_t pid = fork(); // Создание нового процесса

    if (pid == -1) {
        perror("consumer fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Код, выполняемый дочерним процессом
        srand(getpid()); // Инициализация генератора случайных чисел для потребителя
    } else {
        // Код, выполняемый родительским процессом
        consumersPids[consumersCount++] = pid; // Сохранение PID нового потребителя
        return;
    }

    // Получение доступа к разделяемой памяти и семафорам
    int sharedMemoryID;
    Queue* queue;
    initializeSharedMemory(&sharedMemoryID, &queue, 0);

    sem_t *emptySpaceSemaphore;
    sem_t *filledSpaceSemaphore;
    sem_t *mutex;
    openSemaphore(SEM_FREE_SPACE, &emptySpaceSemaphore, QUEUE_CAPACITY);
    openSemaphore(SEM_FILLED_SPACE, &filledSpaceSemaphore, 0);
    openSemaphore(MUTEX, &mutex, 1); // По сути одноместный семафор - мьютекс

    // Основной цикл обработки сообщений потребителя
    while (runConsumer) {
        sem_wait(filledSpaceSemaphore); // Ожидание наличия сообщения для обработки
        sem_wait(mutex); // Захват мьютекса

        printf("Consumer with pid: %d\n", getpid());
        printf(GREEN); // Установка цвета для вывода
        Message message = removeMessageFromQueue(queue); // Извлечение сообщения из очереди
        printMessage(&message); // Вывод содержимого сообщения

        sem_post(mutex); // Освобождение мьютекса
        sem_post(emptySpaceSemaphore); // Увеличение счетчика свободного места в очереди

        printf(WHITE); // Восстановление стандартного цвета вывода
        printf("Count removed messages: %d\n", queue->removedMessages);
        sleep(2); // Пауза перед следующим циклом
    }

    printf(WHITE); // Восстановление стандартного цвета вывода
    munmap(queue, SHM_SIZE); // Отключение разделяемой памяти
    close(sharedMemoryID); // Закрытие дескриптора разделяемой памяти
    closeAllSemaphores(emptySpaceSemaphore, filledSpaceSemaphore, mutex); // Закрытие всех семафоров
    exit(EXIT_SUCCESS); // Завершение процесса потребителя
}

/**
 * @brief Удаляет последнего созданного потребителя.
 */
void deleteConsumer(void) {
    if (consumersCount == 0) {
        printf("There is no consumers to delete\n");
        return;
    }

    printf("Consumer with pid %d was quit\n", consumersPids[consumersCount - 1]);
    kill(consumersPids[--consumersCount], SIGUSR1); // Отправка сигнала завершения потребителю
}

/**
 * @brief Удаляет всех созданных потребителей.
 */
void deleteAllConsumers(void) {
    while (consumersCount > 0) {
        deleteConsumer(); // Последовательное удаление всех потребителей
    }
}
