#define _GNU_SOURCE

#include <errno.h>
#include <locale.h>
#include <regex.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Структура, представляющая информацию о дочернем процессе
struct procesStruct {
    bool forbiddenStatistics;  // статус процесса (разрешена/запрещена
                               // статистика)
    pid_t pid;      // ID процесса
    char name[20];  // название процесса
};

struct procesStruct* childProcesses;  // Массив структур, хранящих информацию о
                                      // дочерних процессах
int childPidsNumber = 0;  // Количество дочерних процессов

// Функция для получения числа из строки
int getChildNum(const char* str) {
    int length = 0;
    int result = 0;

    while (*str) {
        if (*str >= '0' && *str <= '9') {
            result = result * 10 + (*str - '0');
            length++;
        }
        str++;
    }

    return (length > 0) ? result : 0;
}

// Функция для преобразования числа в строку
char* convertNumToStr(int num) {
    int length = 0;
    int temp = num;

    while (temp != 0) {  // количество разрядов
        temp /= 10;
        length++;
    }

    char* str = malloc((length + 1) * sizeof(char));  //выделение памяти
    sprintf(str, "%d", num);
    return str;
}

// Функция для создания дочернего процесса
void createChildProcess(pid_t pid) {
    // Создаем временную строку для преобразования номера дочернего процесса в
    // строку
    char* temp = convertNumToStr(childPidsNumber - 1);

    // Формируем имя для дочернего процесса вида "C_<номер>"
    strcpy(childProcesses[childPidsNumber - 1].name, "C_");
    strcat(childProcesses[childPidsNumber - 1].name, temp);

    // Освобождаем память, выделенную под временную строку
    free(temp);

    // Записываем PID дочернего процесса
    childProcesses[childPidsNumber - 1].pid = pid;

    // Инициализируем статус разрешения статистики (0 означает разрешено)
    childProcesses[childPidsNumber - 1].forbiddenStatistics = 0;
}

// Функция для удаления дочернего процесса
void deleteChildProcess(const struct procesStruct* childProcesses,
                        int processNum) {
    // Выводим сообщение о том, что происходит уничтожение процесса с
    // определенным PID
    printf("killing procces %d...\n", childProcesses[processNum].pid);

    // Отправляем сигнал SIGTERM процессу с указанным PID для завершения его
    // работы
    kill(childProcesses[processNum].pid, SIGTERM);
}

// Функция для удаления всех дочерних процессов
void deleteAllProcesses(void) {
    // Пока количество дочерних процессов не равно нулю
    while (childPidsNumber) {
        // Вызываем функцию deleteChildProcess для удаления последнего дочернего
        // процесса
        deleteChildProcess(childProcesses, childPidsNumber - 1);

        // Уменьшаем счетчик количества дочерних процессов
        childPidsNumber--;
    }

    // После удаления всех дочерних процессов проверяем, что массив
    // childProcesses не является NULL
    if (childProcesses != NULL) {
        // Освобождаем выделенную динамическую память, в которой хранится массив
        // childProcesses
        free(childProcesses);

        // Устанавливаем указатель childProcesses в NULL для предотвращения
        // дальнейшего использования освобожденной памяти
        childProcesses = NULL;
    }
}

// Функция для создания дочернего процесса
void createChild(void) {
    // Создаем новый дочерний процесс с помощью fork()
    pid_t childPid = fork();

    // Увеличиваем счетчик количества дочерних процессов
    childPidsNumber++;

    // Перераспределяем память для массива childProcesses, чтобы добавить место
    // для нового дочернего процесса
    childProcesses =
        realloc(childProcesses, childPidsNumber * sizeof(struct procesStruct));

    // Вызываем функцию createChildProcess для добавления информации о новом
    // дочернем процессе
    createChildProcess(childPid);

    // Проверяем результат создания дочернего процесса
    if (childPid == -1) {
        // Если произошла ошибка при создании дочернего процесса, выводим
        // сообщение об ошибке
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }

    // Если childPid равен 0, это значит, что код выполняется внутри дочернего
    // процесса
    if (childPid == 0) {
        // Запускаем исполнение другой программы (child) внутри дочернего
        // процесса с помощью execve
        char* args[] = {"child", NULL};
        execve("./child", args, NULL);
    } else {
        // Если код выполняется в родительском процессе
        // Выводим информацию о созданном дочернем процессе (его PID и название)
        printf("PID %d (%s) - created\n", childPid,
               childProcesses[childPidsNumber - 1].name);

        // Выводим количество дочерних процессов
        printf("Count of children processes - %d\n", childPidsNumber);
    }
}

// Функция для завершения дочернего процесса
void killChild(void) {
    // Проверяем, что массив childProcesses не равен NULL
    if (childProcesses != NULL) {
        // Удаляем последний дочерний процесс из массива
        deleteChildProcess(childProcesses, childPidsNumber - 1);

        // Уменьшаем счетчик количества дочерних процессов
        childPidsNumber--;

        // Уменьшаем размер массива childProcesses после удаления процесса
        childProcesses = realloc(childProcesses,
                                 childPidsNumber * sizeof(struct procesStruct));

        // Выводим текущее количество дочерних процессов после завершения
        // процесса
        printf("Count of children processes - %d\n", childPidsNumber);
    }
}

// Функция для вывода информации о всех процессах
void printAllProcesses(void) {
    // Проверяем, что массив childProcesses не равен NULL
    if (childProcesses != NULL) {
        // Выводим PID родительского процесса
        printf("parent - %d\n", getpid());

        // Выводим заголовок для дочерних процессов
        printf("child :\n");

        // Перебираем все дочерние процессы в массиве
        for (int i = 0; i < childPidsNumber; i++) {
            // Выводим имя и PID каждого дочернего процесса
            printf("%s - \t%d\t", childProcesses[i].name,
                   childProcesses[i].pid);

            // Проверяем разрешение статистики для текущего дочернего процесса
            if (childProcesses[i].forbiddenStatistics) {
                // Если статистика запрещена, выводим сообщение
                printf("Statistics forbidden!\n");
            } else {
                // Если статистика разрешена, выводим сообщение
                printf("Statistics allowed...\n");
            }
        }
    }
}

// Функция для запрета вывода статистики для всех процессов
void banStatistics(void) {
    // Перебираем все дочерние процессы в массиве
    for (int i = 0; i < childPidsNumber; i++) {
        // Отправляем сигнал SIGUSR1 процессу с PID из массива
        kill(childProcesses[i].pid, SIGUSR1);

        // Устанавливаем флаг forbiddenStatistics в true для текущего процесса
        childProcesses[i].forbiddenStatistics = true;
    }
}

// Функция для разрешения вывода статистики для всех процессов
void allowStatistics(void) {
    // Перебираем все дочерние процессы в массиве
    for (int i = 0; i < childPidsNumber; i++) {
        // Отправляем сигнал SIGUSR2 процессу с PID из массива
        kill(childProcesses[i].pid, SIGUSR2);

        // Устанавливаем флаг forbiddenStatistics в false для текущего процесса
        childProcesses[i].forbiddenStatistics = false;
    }
}

// Функция для отправки сигнала всем дочерним процессам
void sendSignalToAllChilds(int signal) {
    // Устанавливаем флаг запрета статистики по умолчанию в true
    bool forbid = true;

    // Если сигнал равен SIGUSR2, устанавливаем флаг запрета в false
    if (signal == SIGUSR2) {
        forbid = false;
    }

    // Перебираем все дочерние процессы в массиве
    for (int i = 0; i < childPidsNumber; i++) {
        // Отправляем указанный сигнал процессу с PID из массива
        kill(childProcesses[i].pid, signal);

        // Устанавливаем флаг forbiddenStatistics в соответствии с запретом
        childProcesses[i].forbiddenStatistics = forbid;
    }
}

// Функция для остановки всех процессов и вывода статистики определенного
// процесса
void stopAllAndRequest(int childPidNum) {
    // Отправляем сигнал SIGUSR1 всем дочерним процессам для остановки вывода
    // статистики
    sendSignalToAllChilds(SIGUSR1);

    // Задержка на 1000 микросекунд (1 миллисекунда)
    usleep(1000);

    // Отправляем сигнал SIGUSR2 определенному дочернему процессу для запроса
    // статистики
    kill(childProcesses[childPidNum].pid, SIGUSR2);

    // Устанавливаем флаг forbiddenStatistics для данного процесса в false
    // (разрешаем статистику)
    childProcesses[childPidNum].forbiddenStatistics = false;

    // Задержка на 2000 микросекунд (2 миллисекунды)
    usleep(2000);

    // Отправляем сигнал SIGURG для запроса статистики определенного процесса
    kill(childProcesses[childPidNum].pid, SIGURG);

    // Выводим сообщение о том, что процесс запущен
    printf("%s is running\n", childProcesses[childPidNum].name);

    // Задержка на 20 секунд для выполнения процессом своей работы
    sleep(20);

    // Отправляем сигнал SIGUSR2 всем дочерним процессам для возобновления
    // вывода статистики
    sendSignalToAllChilds(SIGUSR2);
}

// Функция для проверки ввода пользователя
int checkInput(const char* option) {
    const char* pattern1 = "[sgp]<[0-9]+>";
    const char* pattern2 = "[qlksg+-]";
    regex_t regex1, regex2;
    int ret = regcomp(&regex1, pattern1, REG_EXTENDED);
    if (ret) {
        printf("Error in regex 1\n");
        regfree(&regex1);
        return 1;
    }

    ret = regcomp(&regex2, pattern2, REG_EXTENDED);
    if (ret) {
        printf("Error in regex 2\n");
        regfree(&regex2);
        return 1;
    }

    ret = regexec(&regex1, option, 0, NULL, 0);
    if (!ret) {
        return 1;
    }
    if (ret == REG_NOMATCH) {
        ret = regexec(&regex2, option, 0, NULL, 0);
        if (!ret) {
            return 2;
        }
        return -1;
    }
    return -1;
}

void printOptions(void) {
    printf("\n\n\nOptions:\n");
    printf("  + : Create a new child process\n");
    printf("  - : Kill the last created child process\n");
    printf("  q : Delete all processes and exit\n");
    printf("  l : Print information about all child processes\n");
    printf("  k : Kill all child processes\n");
    printf("  s : Ban statistics for all child process\n");
    printf("  g : Allow statistics for all child process\n");
    printf("  s<num> : Stop statistics for child <num>\n");
    printf("  g<num> : Allow statistics for child <num>\n");
    printf("  p<num> : Stop all and request information from child <num>\n");
    printf("Enter option: ");
}
int main(void) {
    while (true) {
        printOptions();

        char option[10];
        strcpy(option, "\0");
        fgets(option, sizeof(option), stdin);  // Ввод опции с клавиатуры

        int regexCheck = checkInput(
            option);  // Проверка ввода на соответствие определенным шаблонам

        if (regexCheck == 1) {  // Если ввод соответствует шаблону "Х<num>"
            int childNum = getChildNum(
                option);  // Получаем номер дочернего процесса из опции

            if (childNum >= childPidsNumber) {  // Проверяем корректность номера
                                                // дочернего процесса
                printf("Error - %d\n", childPidsNumber);
                continue;  // Пропускаем итерацию цикла, если номер некорректен
            }

            // Обработка команд в зависимости от первого символа опции
            if (option[0] == 's') {
                // Остановить вывод статистики для выбранного дочернего процесса
                kill(childProcesses[childNum].pid, SIGUSR1);
                childProcesses[childNum].forbiddenStatistics = true;
                printf("%s statistics is forbidden\n",
                       childProcesses[childNum].name);
            } else if (option[0] == 'g') {
                // Разрешить вывод статистики для выбранного дочернего процесса
                kill(childProcesses[childNum].pid, SIGUSR2);
                childProcesses[childNum].forbiddenStatistics = false;
                printf("%s statistics is allowed\n",
                       childProcesses[childNum].name);
            } else if (option[0] == 'p') {
                // Остановить все процессы и запросить статистику для
                // определенного процесса
                stopAllAndRequest(childNum);
            }
        } else if (regexCheck ==
                   2) {  // Если ввод соответствует другому шаблону
            switch (option[0]) {
                case '+':
                    // Создать новый дочерний процесс
                    createChild();
                    break;
                case '-':
                    // Завершить последний созданный дочерний процесс
                    killChild();
                    break;
                case 'q':
                    // Удалить все процессы и завершить программу
                    deleteAllProcesses();
                    exit(0);
                case 'l':
                    // Вывести информацию о всех дочерних процессах
                    printAllProcesses();
                    break;
                case 'k':
                    // Удалить все дочерние процессы
                    deleteAllProcesses();
                    break;
                case 's':
                    // Запретить вывод статистики для всех процессов
                    banStatistics();
                    break;
                case 'g':
                    // Разрешить вывод статистики для всех процессов
                    allowStatistics();
                    break;
                default:
                    break;
            }
        } else {
            puts("Wrong input");  // Вывод сообщения об ошибке при некорректном
                                  // вводе
        }
    }

    return 0;
}
