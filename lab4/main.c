#include "functions/functions.h"

/**
 * @brief Основная функция программы.
 *
 * Осуществляет инициализацию разделяемой памяти, семафоров и очереди,
 * устанавливает обработчики сигналов, запускает главное меню обработки,
 * а затем закрывает ресурсы и завершает работу программы.
 *
 * @return Код возврата.
 */
int main(void) {
    int sharedMemoryId;
    Queue* queue;
    sem_t* emptySpaceCount;
    sem_t* filledSpaceCount;
    sem_t* queueMutex;

    // Инициализация разделяемой памяти и связывание с очередью
    initializeSharedMemory(&sharedMemoryId, &queue, 1);

    // Инициализация всех семафоров
    initializeAllSemaphores(&emptySpaceCount,
                            &filledSpaceCount,
                            &queueMutex);

    // Инициализация очереди
    initializeQueue(queue);

    // Установка обработчиков сигналов
    initializeHandler();

    // Запуск главного меню обработки с передачей идентификатора разделяемой памяти и указателя на очередь
    menu(sharedMemoryId, queue);

    // Закрытие всех семафоров
    closeAllSemaphores(emptySpaceCount, filledSpaceCount, queueMutex);

    // Очистка ресурсов (удаление разделяемой памяти и семафоров)
    cleanResources();

    return 0; // Возврат успешного завершения работы программы
}
