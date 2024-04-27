#include <stdio.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ENV_NUM 7

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

bool search_child(char** envp, const char* name_variable) {
    size_t ind = 0;
    size_t len = strlen(name_variable);
    bool flag_find = false;
    while(envp[ind]) {
        flag_find = true;
        for (size_t i = 0; i < len; ++i) {
            if (envp[ind][i] != name_variable[i]) {
                flag_find = false;
                break;
            }
        }
        if (flag_find == true)
            break;
        ind++;
    }
    if (flag_find == false) return false;
    size_t path_len = strlen(name_variable);
    size_t find_str_len = strlen(envp[ind]);
    for (size_t i = path_len + 1; i < find_str_len; ++i) {
        printf("%c", envp[ind][i]);
    }
    printf("\n");
    return true;
}

int main(int argc, char* argv[], char* envp[]) {
    printf("Program name: %s\n", argv[0]);
    printf("Program PID: %d\n", getpid());
    printf("Program parent PID: %d\n", getppid());
    char** env_names = (char**)calloc(ENV_NUM, sizeof(char*));
    get_env_names(argv[1], &env_names);
    switch(argv[2][0]) {
        case('+'): {
            for (size_t i = 0; i < ENV_NUM; i++) {
                printf("%s\n", getenv(env_names[i]));
            }
            break;
        }
        case('*'): {
            for(size_t i = 0; i < ENV_NUM; i++)
                search_child(envp, env_names[i]);
            break;
        }
        case('&'): {
            for(size_t i = 0; i < ENV_NUM; i++)
                search_child(envp, env_names[i]);
            break;
        }
    }
    for (size_t i = 0; i < ENV_NUM; i++)
        free(env_names[i]);
    free(env_names);
    return 0;
}
