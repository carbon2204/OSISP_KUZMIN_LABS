#define _POSIX_C_SOURCE 199309L
#include "onSignal.h"
#include "timer.h"
#include <memory.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

bool silent = false;

struct Hole
{
    int a, b;
} hole = {0};

struct Statistic
{
    int down_down, down_up, up_down, up_up;
} statistic = {0};

void reset()
{
    memset(&hole, 0, sizeof(hole));
    memset(&statistic, 0, sizeof(statistic));
}

void collect()
{
    struct Hole copy = hole;
    if (copy.a && copy.b)
        statistic.up_up++;
    if (copy.a && !copy.b)
        statistic.up_down++;
    if (!copy.a && copy.b)
        statistic.down_up++;
    if (!copy.a && !copy.b)
        statistic.down_down++;
}

void print()
{
    struct Statistic copy = statistic;
    printf(
        "PPID:%d PID:%5d 00:%4d 01:%4d 10:%4d 11:%4d\n",
        getppid(),
        getpid(),
        copy.down_down,
        copy.down_up,
        copy.up_down,
        copy.up_up);
}

void onTimer() { collect(); }

void onUsr1() { silent = true; }

void onUsr2() { silent = false; }

int main()
{
    onSignal(SIGUSR1, onUsr1);
    onSignal(SIGUSR2, onUsr2);
    startIntervalTimer(createTimer(onTimer), us(100));

    while (true)
    {
        reset();
        for (int i = 0; i < 100000000; i++)
        {
            hole.a = 0;
            hole.b = 0;
            hole.a = 1;
            hole.b = 1;
        }
        if (!silent)
            print();
    }
}