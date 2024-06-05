#ifdef __APPLE__

#ifndef BARRIER_H_
#define BARRIER_H_

#include <pthread.h>
#include <errno.h>

typedef int pthread_barrierattr_t;
typedef struct
{
    pthread_mutex_t mutex; // мьютекс для синхронизации потоков
    pthread_cond_t cond; // условная переменная для ожидания события
    int count; // счетчик потоков
    int tripCount; // количество потоков, необходимых для срабатывания барьера
} pthread_barrier_t;


int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)
{
    if(count == 0) // проверка на некорректное количество потоков
    {
        errno = EINVAL; // установка кода ошибки
        return -1; // возвращаем ошибку
    }
    if(pthread_mutex_init(&barrier->mutex, 0) < 0) // инициализация мьютекса
    {
        return -1; // возвращаем ошибку
    }
    if(pthread_cond_init(&barrier->cond, 0) < 0) // инициализация условной переменной
    {
        pthread_mutex_destroy(&barrier->mutex); // уничтожение мьютекса
        return -1; // возвращаем ошибку
    }
    barrier->tripCount = count; // установка количества потоков для срабатывания барьера
    barrier->count = 0; // инициализация счетчика потоков

    return 0; // успешное выполнение
}

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    pthread_cond_destroy(&barrier->cond); // уничтожение условной переменной
    pthread_mutex_destroy(&barrier->mutex); // уничтожение мьютекса
    return 0; // успешное выполнение
}

int pthread_barrier_wait(pthread_barrier_t *barrier)
{
    pthread_mutex_lock(&barrier->mutex); // блокировка мьютекса
    ++(barrier->count); // увеличение счетчика потоков
    if(barrier->count >= barrier->tripCount) // проверка достижения необходимого количества потоков
    {
        barrier->count = 0; // сброс счетчика потоков
        pthread_cond_broadcast(&barrier->cond); // сигнализация всем потокам о событии
        pthread_mutex_unlock(&barrier->mutex); // разблокировка мьютекса
        return 1; // возвращаем флаг срабатывания барьера
    }
    else
    {
        pthread_cond_wait(&barrier->cond, &(barrier->mutex)); // ожидание события
        pthread_mutex_unlock(&barrier->mutex); // разблокировка мьютекса
        return 0; // возвращаем флаг ожидания
    }
}

#endif // PTHREAD_BARRIER_H_
#endif // __APPLE__