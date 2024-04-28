#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "onSignal.h"

pid_t *children = NULL;
size_t childrenSize = 0;

enum Action
{
    CREATE,
    KILL,
    LIST,
    KILL_ALL,
    SILENT,
    GARRULOUS,
    PRIORITIZE,
    EXIT,
    UNKNOWN_ACTION
};

struct ParsedAction
{
    enum Action action;
    int num;
};

enum Action charToAction(char c)
{
    switch (c)
    {
    case '+':
        return CREATE;
    case '-':
        return KILL;
    case 'l':
        return LIST;
    case 'k':
        return KILL_ALL;
    case 's':
        return SILENT;
    case 'g':
        return GARRULOUS;
    case 'p':
        return PRIORITIZE;
    case 'q':
        return EXIT;
    default:
        return UNKNOWN_ACTION;
    }
}

struct ParsedAction parse(char *input)
{
    enum Action action = charToAction(*input);
    int num = atoi(input + 1);

    struct ParsedAction result = {action, num};

    return result;
}

void createChild()
{
    childrenSize++;
    children = realloc(children, sizeof(pid_t) * childrenSize);

    pid_t pid = fork();

    if (!pid)
        execv("./child", (char *const *){NULL});

    children[childrenSize - 1] = pid;

    printf("Created C_%lu: PID=%d\n", childrenSize, pid);
}

void killLatest()
{
    if (!childrenSize)
    {
        printf("No processes to kill.\n");
        return;
    }

    childrenSize--;

    kill(children[childrenSize], SIGKILL);
    waitpid(children[childrenSize], NULL, 0);

    printf("Killed C_%lu (PID=%d)\n", childrenSize + 1, children[childrenSize]);

    children = realloc(children, sizeof(pid_t) * childrenSize);
}

void listAll()
{
    for (size_t i = 0; i < childrenSize; i++)
    {
        printf("C_%lu (PID=%d)\n", i + 1, children[i]);
    }
}

void killAll()
{
    for (size_t i = 0; i < childrenSize; i++)
    {
        kill(children[i], SIGKILL);
        waitpid(children[i], NULL, 0);
        printf("Killed C_%lu (PID=%d)\n", i + 1, children[i]);
    }

    childrenSize = 0;
    free(children);
    children = NULL;
}

void silent(size_t index)
{
    if (index == 0)
    {
        for (size_t i = 0; i < childrenSize; i++)
        {
            kill(children[i], SIGUSR1);
            printf("Muted C_%ld\n", i + 1);
        }
        return;
    }

    if (index > 0 && index <= childrenSize)
    {
        kill(children[index - 1], SIGUSR1);
        printf("Muted C_%ld\n", index);
        return;
    }

    printf("C_%ld not found\n", index);
}

void garrulous(size_t index)
{
    if (index == 0)
    {
        alarm(0);
        for (size_t i = 0; i < childrenSize; i++)
        {
            kill(children[i], SIGUSR2);
            printf("Unmuted C_%ld\n", i + 1);
        }
        return;
    }

    if (index > 0 && index <= childrenSize)
    {
        kill(children[index - 1], SIGUSR2);
        printf("Unmuted C_%ld\n", index);
        return;
    }

    printf("C_%ld not found\n", index);
}

void ask(size_t index)
{
    alarm(5);
    for (size_t i = 0; i < childrenSize; i++)
    {
        kill(children[i], (i + 1 == index) ? SIGUSR2 : SIGUSR1);
        printf((i == index - 1) ? "Unmuted C_%ld\n" : "Muted C_%ld\n", i + 1);
    }

    printf("C_%ld not found\n", index);
}

void handleAction(struct ParsedAction action)
{
    switch (action.action)
    {
    case CREATE:
        createChild();
        break;
    case KILL:
        killLatest();
        break;
    case LIST:
        listAll();
        break;
    case KILL_ALL:
        killAll();
        break;
    case SILENT:
        silent(action.num);
        break;
    case GARRULOUS:
        garrulous(action.num);
        break;
    case PRIORITIZE:
        ask(action.num);
        break;
    case EXIT:
        killAll();
        exit(0);
    case UNKNOWN_ACTION:
        printf("Unknown action\n");
        break;
    }
}

void onAlarm(int a)
{
    (void)a;
    garrulous(0);
}

int main()
{
    onSignal(SIGALRM, onAlarm);
    char buff[256];

    while (1)
    {
        fgets(buff, sizeof(buff), stdin);
        rewind(stdin);
        handleAction(parse(buff));
    }
}