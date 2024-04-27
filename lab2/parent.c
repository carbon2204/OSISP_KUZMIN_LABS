#include <stdio.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define ENV_NUM 7
#define CHILD_PATH "CHILD_PATH"

int compare(const void* a, const void* b) {
    return strcoll((const char*)a, (const char*)b);
}

void get_env_names(const char* path, char*** buffer) {
    FILE* file = NULL;
    if ((file = fopen(path, "r")) != NULL) {
        size_t ind = 0;
        size_t cnt = ENV_NUM;
        while(cnt--) {
            (*buffer)[ind] = (char*)realloc((*buffer)[ind], 256 * sizeof(char));
            fscanf(file, "%s", (*buffer)[ind++]);
        }
        fclose(file);
    }
    perror("File error");
}

void child_path_search(char** envp, char** path_child) {
    size_t ind = 0;
    size_t len = strlen(CHILD_PATH);
    bool search_flag = false;
    while(envp[ind]) {
        search_flag = true;
        for (size_t i = 0; i < len; ++i) {
            if (envp[ind][i] != CHILD_PATH[i]) {
                search_flag = false;
                break;
            }
        }
        if (search_flag == true)
            break;
        ind++;
    }
    if (search_flag == false) return;
    printf("%s\n", envp[ind]);
    size_t path_len = strlen(CHILD_PATH);
    size_t find_str_len = strlen(envp[ind]);
    *path_child = (char*)malloc(find_str_len - (path_len+1) + 1);
    size_t j = 0;
    for (size_t i = path_len + 1; i < find_str_len; ++i) {
        (*path_child)[j++] = envp[ind][i];
    }
    path_child[ind] = '\0';
}

void child_inc(char** file_name) {
    if ((*file_name)[7] == '9') {
        (*file_name)[7] = '0';
        (*file_name)[6]++;
        return;
    }
    (*file_name)[7]++;
}

int main(int argc, char* argv[], char* envp[]) {
    setlocale(LC_COLLATE, "C");
    char* path = argv[1];
    char** env_names = (char**)calloc(ENV_NUM, sizeof(char*));
    get_env_names(path, &env_names);
    qsort(env_names, ENV_NUM, sizeof(char*), compare);
    for (size_t i = 0; i < ENV_NUM; ++i)
        printf("%s\n", env_names[i]);
    char* child_name = (char*)malloc(9);
    strcpy(child_name, "child_00");
    char* child_args[] = {child_name, path, NULL, NULL };
    bool flag = true;
    do {
        int ch = getchar();
        switch (ch) {
            case '+' : {
                pid_t pid = fork();
                child_args[2] = "+";
                if (pid == 0) {
                    const char *path_child = getenv(CHILD_PATH);
                    execve(path_child, child_args, envp);
                } else {
                    child_inc(&child_name);
                }
                break;
            }
            case '*' : {
                pid_t pid = fork();
                if (pid == 0) {
                    child_args[2] = "*";
                    char *path_child = NULL;
                    child_path_search(envp, &path_child);
                    execve(path_child, child_args, envp);
                } else {
                    child_inc(&child_name);
                }
                break;
            }
            case '&' : {
                pid_t pid = fork();
                if (pid == 0) {
                    child_args[2] = "&";
                    char *path_child = NULL;
                    child_path_search(__environ, &path_child);
                    execve(path_child, child_args, envp);
                } else {
                    child_inc(&child_name);
                }
                break;
            }
            default: {
                flag = false;
                break;
            }
        }
        waitpid(-1, NULL, WNOHANG);
        getchar();
    } while(flag);
    free(child_name);
    for (size_t i = 0; i < ENV_NUM; ++i)
        free(env_names[i]);
    free(env_names);
    return 0;
}
