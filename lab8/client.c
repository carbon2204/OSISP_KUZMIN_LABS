#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define SERVER_ADDRESS "127.0.0.1"

char dir[PATH_MAX];

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("Usage: ./client <port>\n");
        return -1;
    }

    int socket_fd, valread;
    int port = atoi(argv[1]);
    if (port == 0) {
        perror("Port");
        return -1;
    }

    struct sockaddr_in server_address;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_address.sin_port = htons(port);

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connect");
        return -1;
    }

    strcpy(dir, "");

    char *f = NULL;
    int list_count, flag = 1;
    struct dirent **namelist;
    char lnk[1024], buffer[1024];
    mode_t link_to;
    FILE *file;
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        check:
        if (flag) {
            printf("%s>", dir);
            fgets(buffer, sizeof(buffer), stdin);
        } else {
            if (fgets(buffer, 100, file) == NULL) {
                flag = 1;
                fclose(file);
                goto check;
            }
        }
        if (buffer[0] == '@') {
            f = buffer + 1;
            strtok(f, "\n");
            file = fopen(f, "r");
            if (file == NULL) {
                perror("File open");
            } else {
                flag = 0;
                if (fgets(buffer, 100, file) == NULL) {
                    flag = 1;
                    fclose(file);
                    goto check;
                }
            }
        }

        send(socket_fd, buffer, strlen(buffer), 0);

        if (strncasecmp(buffer, "CD ", strlen("CD ")) == 0) {
            memset(buffer, 0, sizeof(buffer));
            valread = recv(socket_fd, buffer, sizeof(buffer), 0);
            if (strcmp(buffer, "/") == 0) {
                puts("Wrong dir");
            } else if (strcmp(buffer, "//") == 0) {
                printf("");
            } else {
                memset(dir, 0, sizeof(dir));
                if (strcmp(buffer, ".") != 0)
                    strcpy(dir, buffer);
            }

        } else if (strcasecmp(buffer, "LIST\n") == 0) {
            valread = recv(socket_fd, &list_count, sizeof(int), 0);
            printf("Num: %d\n", list_count);
            namelist = (struct dirent **)malloc(sizeof(struct dirent *) * list_count);

            for (int i = 0; i < list_count; i++) {
                namelist[i] = (struct dirent *)malloc(sizeof(struct dirent));
                valread = recv(socket_fd, namelist[i], sizeof(struct dirent), 0);
                unsigned char type = namelist[i]->d_type;

                if (type == DT_DIR) {
                    printf("%s/\n", namelist[i]->d_name);
                } else if (type == DT_LNK) {
                    memset(&link_to, 0, sizeof(link_to));
                    valread = recv(socket_fd, &link_to, sizeof(mode_t), 0);

                    // Получение названия объекта, на который указывает ссылка
                    memset(lnk, 0, sizeof(lnk));
                    valread = recv(socket_fd, lnk, sizeof(lnk), 0);

                    if (S_ISLNK(link_to)) {
                        printf("%s -->> %s\n", namelist[i]->d_name, lnk);
                    } else if (S_ISREG(link_to)) {
                        printf("%s --> %s\n", namelist[i]->d_name, lnk);
                    } else {  // Другие типы
                        printf("%s -- %s\n", namelist[i]->d_name, lnk);
                    }
                } else {  // Вывод всего остального
                    printf("%s\n", namelist[i]->d_name);
                }

                free(namelist[i]);
            }
            free(namelist);
        } else if (strcasecmp(buffer, "INFO\n") == 0) {
            char out[256] = {'\0'};
            memset(out, 0, sizeof(out));
            recv(socket_fd, out, sizeof(out), 0);
            printf("%s", out);
        } else {

            memset(buffer, 0, sizeof(buffer));
            valread = recv(socket_fd, buffer, sizeof(buffer), 0);

            printf("%s\n", buffer);
        }

        if (strcasecmp(buffer, "BYE") == 0) {
            close(socket_fd);
            break;
        }
    }

    return 0;
}