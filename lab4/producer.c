#include "producer.h"
#include "../functions/functions.h"
#include <sys/mman.h>

// Глобальные переменные для отслеживания состояния производителей
volatile sig_atomic_t runProducer = 1;
pid_t producersPids[PRODUCERS_MAX_COUNT];
size_t producersCount = 0;

/**
 * @brief Создает нового производителя (процесс-потомок).
 *
 * Функция создает новый процесс-производитель с помощью fork().
 * Дочерний процесс занимается добавлением сообщений в общую очередь.
 */
void createProducer(void) {
    if (producersCount == PRODUCERS_MAX_COUNT) {
        printf("Max count of producers is reached\n");
        return;
    }
    
    pid_t pid = fork(); // Создание нового процесса

    if (pid == -1) {
        perror("producer fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        srand(getpid()); // Инициализация генератора случайных чисел для потомка
    } else {
        producersPids[producersCount++] = pid; // Сохранение PID нового производителя
        return;
    }

    // Получение доступа к разделяемой памяти и семафорам
    int sharedMemoryID;
    Queue* queue;
    initializeSharedMemory(&sharedMemoryID, &queue, 0);

    sem_t *emptySpaceCount;
    sem_t *filledSpaceCount;
    sem_t *mutex;
    openSemaphore(SEM_FREE_SPACE, &emptySpaceCount, QUEUE_CAPACITY);
    openSemaphore(SEM_FILLED_SPACE, &filledSpaceCount, 0);
    openSemaphore(MUTEX, &mutex, 1);

    // Основной цикл производства сообщений
    while (runProducer) {
        sem_wait(emptySpaceCount); // Ожидание свободного места в очереди
        sem_wait(mutex); // Захват мьютекса

        Message *message = createMessage(); // Создание нового сообщения
        addMessageToQueue(queue, message); // Добавление сообщения в очередь

        printf("Producer with pid: %d\n", getpid());
        printf(CIAN); // Установка цвета для вывода
        printMessage(message); // Вывод информации о сообщении

        sem_post(mutex); // Освобождение мьютекса
        sem_post(filledSpaceCount); // Увеличение счетчика заполненных мест в очереди
        printf(WHITE); // Восстановление стандартного цвета вывода

        printf("Count added messages: %u\n", queue->addedMessages);
        free(message); // Освобождение памяти, выделенной под сообщение
        sleep(4); // Пауза перед созданием следующего сообщения
    }

    printf(WHITE); // Восстановление стандартного цвета вывода
    munmap(queue, SHM_SIZE); // Отключение разделяемой памяти
    close(sharedMemoryID); // Закрытие дескриптора разделяемой памяти
    closeAllSemaphores(emptySpaceCount, filledSpaceCount, mutex); // Закрытие всех семафоров
    exit(EXIT_SUCCESS); // Завершение процесса производителя
}

/**
 * @brief Удаляет последнего созданного производителя.
 */
void deleteProducer(void) {
    if (producersCount == 0) {
        printf("There are no producers to delete\n");
        return;
    }

    printf("Producer with pid %d was quit\n", producersPids[producersCount - 1]);
    kill(producersPids[--producersCount], SIGUSR2); // Отправка сигнала завершения производителю
}

/**
 * @brief Удаляет всех созданных производителей.
 */
void deleteAllProducers(void) {
    while (producersCount > 0) {
        deleteProducer(); // Последовательное удаление всех производителей
    }
}
