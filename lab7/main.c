#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define MAX_RECORDS 10

typedef struct record_s {
    char name[80];
    char address[80];
    int semester;
} record_t;

int fd;
struct flock lock;

void print_record(int rec_no) {
    record_t record;
    off_t offset = rec_no * sizeof(record);
    lseek(fd, offset, SEEK_SET);
    read(fd, &record, sizeof(record));
    printf("%d: %s, %s, %d\n", rec_no, record.name, record.address, record.semester);
}

void get_record(int rec_no, record_t *record) {
    off_t offset = rec_no * sizeof(*record);
    lseek(fd, offset, SEEK_SET);
    read(fd, record, sizeof(*record));
}

void modify_record(int rec_no, record_t *record) {
    off_t offset = rec_no * sizeof(*record);
    lseek(fd, offset, SEEK_SET);
    write(fd, record, sizeof(*record));
}

void save_record(record_t *record, record_t *new_record, int rec_no) {
    if (rec_no < 0 || rec_no >= MAX_RECORDS) {
        printf("Invalid record number\n");
        return;
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = rec_no * sizeof(*record);
    lock.l_len = sizeof(*record);
    while (fcntl(fd, F_SETLK, &lock) == -1) {
        if (errno == EAGAIN) {
            printf("Record %d is locked. Waiting...\n", rec_no);
            sleep(1);
        } else {
            perror("fcntl");
            exit(1);
        }
    }
    sleep(2);

    record_t current_record;
    lseek(fd, rec_no * sizeof(current_record), SEEK_SET);
    if (read(fd, &current_record, sizeof(current_record)) == -1) {
        perror("read");
        return;
    }

    if (strcmp(current_record.name, record->name) != 0 || strcmp(current_record.address, record->address) != 0 || current_record.semester != record->semester) {
        printf("Record %d has been modified by another process. Trying again...\n", rec_no);
        get_record(rec_no, record);
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        save_record(record, new_record, rec_no);
    } else {
        printf("Writing...\n");
        modify_record(rec_no, new_record);
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
}

void fileCreate(const char *filename) {
    struct record_s arrayRecords[10];

    for (int i = 0; i < 10; i++) {
        sprintf(arrayRecords[i].name, "Name_%c", 'A' + i);
        sprintf(arrayRecords[i].address, "Address%d", i);
        arrayRecords[i].semester = i % 2 + 1;
    }
    FILE *file;
    if ((file = fopen(filename, "r")) != NULL) {
        fclose(file);
        return;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        perror("file");
        return;
    }
    fwrite(arrayRecords, sizeof(arrayRecords), 1, file);
    fclose(file);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: ./main <filename>");
        return -1;
    }

    fileCreate(argv[1]);

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("Filename");
        exit(1);
    }

    printf("File opened successfully\n");

    printf("Menu:\nLST - list all records\nGET - get a record\nPUT - save the last modified record\nEXIT - exit the program\n");

    char command[4];
    int rec_no = -1;
    record_t current_record;

    while (1) {
        printf(">");
        scanf("%s", command);
        if (strcasecmp(command, "LST") == 0) {
            printf("Records:\n");
            for (int i = 0; i < MAX_RECORDS; i++) {
                print_record(i);
            }
        } else if (strcmp(command, "GET") == 0) {
            printf("Record number: ");
            scanf("%d", &rec_no);

            get_record(rec_no, &current_record);

            printf("Record %d: %s, %s, %d\n", rec_no, current_record.name, current_record.address, current_record.semester);
        } else if (strcasecmp(command, "PUT") == 0) {
            if (rec_no == -1) {
                printf("No records was getted\n");
                continue;
            }

            record_t new_record;

            get_record(rec_no, &current_record);

            printf("Name: ");
            scanf("%s", new_record.name);

            printf("Address: ");
            scanf("%s", new_record.address);

            printf("Semester: ");
            scanf("%d", &new_record.semester);

            save_record(&current_record, &new_record, rec_no);

            printf("Record %d saved successfully\n", rec_no);

        } else if (strcasecmp(command, "EXIT") == 0) {
            printf("Exiting program...\n");
            break;
        } else {
            printf("Invalid command\n");
        }
    }
    close(fd);
    return 0;
}