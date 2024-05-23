#define _GNU_SOURCE
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define CICLE_COUNT 101

bool info = 1;  // Доступ (флаг для разрешения/запрета вывода информации)

// Счетчики для различных комбинаций значений пары
int count00 = 0;  
int count01 = 0;
int count10 = 0;
int count11 = 0;

struct Pair {
    int first;
    int second;
};

struct Pair pair;  // Переменная для хранения пары значений

bool isSignal = 0;  // Флаг для проверки получения сигнала

// Обработчик сигнала SIGALRM - срабатывает по истечении установленного времени
// таймера, обрабатывает текущую пару значений
void alarmSignalHandler(int signal) {
    if (signal == SIGALRM) {
        // Обработка пары значений (увеличивает счетчик для информации)
        if (pair.first == 0 && pair.second == 0) {
            count00++;
        } else if (pair.first == 0 && pair.second == 1) {
            count01++;
        } else if (pair.first == 1 && pair.second == 0) {
            count10++;
        } else if (pair.first == 1 && pair.second == 1) {
            count11++;
        }
        isSignal = 1;  // Устанавливает флаг получения сигнала
    }
}

// Обработчик сигнала SIGUSR1 и SIGUSR2
void usrSignalHandler(int signal) {
    if (signal == SIGUSR1) {
        info = false;  // Запрет вывода информации
        isSignal = true;  // Устанавливает флаг получения сигнала
    } else if (signal == SIGUSR2) {
        info = true;  // Разрешает вывод информации
        isSignal = true;  // Устанавливает флаг получения сигнала
    }
}

// Обработчик сигнала SIGURG
void urgSignalHandler(int signal) {
    // Вывод информации о текущем состоянии счетчиков
    printf(
        "\nChild process: PID = %d, PPID = %d, 00: %d, 01: %d, 10: %d, 11: %d",
        getpid(), getppid(), count00, count01, count10, count11);
}

int main(void) {
    srand(time(NULL));  // Инициализация генератора случайных чисел

    // Установка обработчиков сигналов
    signal(SIGUSR1, usrSignalHandler);
    signal(SIGUSR2, usrSignalHandler);
    signal(SIGALRM, alarmSignalHandler);
    signal(SIGURG, urgSignalHandler);

    while (1) {
        for (int i = 0; i < CICLE_COUNT; ++i) {
            ualarm(50000,
                   0);  // Установка таймера для генерации сигнала SIGALRM
            while (!isSignal) {
                usleep(9500 +
                       rand() %
                           3000);  // Задержка перед генерацией пары значений
                if (pair.first == 0) {
                    pair.first =
                        1;  // Генерация случайного значения для первого бита
                    usleep(8000 +
                           rand() %
                               3000);  // Задержка перед генерацией второго бита
                    pair.second =
                        1;  // Генерация случайного значения для второго бита
                } else {
                    pair.first =
                        0;  // Генерация случайного значения для первого бита
                    usleep(8000 +
                           rand() %
                               3000);  // Задержка перед генерацией второго бита
                    pair.second =
                        0;  // Генерация случайного значения для второго бита
                }
            }
            isSignal = 0;  // Сброс флага получения сигнала
        }

        if (info) {
            // Вывод информации о текущем состоянии счетчиков, если разрешен
            // вывод информации
            printf(
                "\nChild process: PID = %d, PPID = %d, 00: %d, 01: %d, 10: %d, "
                "11: %d",
                getpid(), getppid(), count00, count01, count10, count11);
        }
    }

    return 0;
}

