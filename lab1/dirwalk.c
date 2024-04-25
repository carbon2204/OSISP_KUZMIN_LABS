#define PATH_MAX 2048
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <locale.h>

struct options {
    int show_files;
    int show_dirs;
    int show_links;
    int sort;
};

struct option long_options[] = {
        {"show_links", no_argument, 0, 'l'},
        {"show_dirs", no_argument, 0, 'd'},
        {"show_files", no_argument, 0, 'f'},
        {"sort", no_argument, 0, 's'},
};


int comparer(const struct dirent** first, const struct dirent** second) {
    return strcoll((*first)->d_name, (*second)->d_name);
}

void dirwalk(char *dir_path,  struct options *opts) {
    struct dirent** entries;

    int res;
    res = scandir(dir_path, &entries, NULL, opts->sort ? &comparer : NULL);

    if (res == -1) {
        printf("Error: opening directory - %s\n", dir_path);
        return;
    }

    for (int i = 0; i < res; ++i) {
        struct dirent *entry = entries[i];
        char filePath[PATH_MAX];

        snprintf(filePath, PATH_MAX, "%s/%s", dir_path, entry->d_name);

        struct stat sb;
        if (lstat(filePath, &sb) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(sb.st_mode) &&
            (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0))
            continue;

        if (S_ISDIR(sb.st_mode)){
            char* new_path = calloc(strlen(dir_path) + strlen(entry->d_name) + 2,sizeof(char));
            strcpy(new_path, dir_path);
            strcat(new_path, "/");
            strcat(new_path, entry->d_name);
            dirwalk(new_path, opts);
            free(new_path);
        }

        if (opts->show_files == 1 && S_ISREG(sb.st_mode)) {
            printf("File: %s\n", entry->d_name);
        }

        else if(opts->show_dirs == 1 && S_ISDIR(sb.st_mode)) {
            printf("Directory: %s\n", entry->d_name);
        }

        else if(opts->show_links && S_ISLNK(sb.st_mode)) {
            printf("Link: %s\n", entry->d_name);
        }
        free(entry);
    }
    free(entries);
}


int main(int argc, char *argv[]) {
    setlocale(LC_COLLATE, "");
    struct options opts = {0, 0, 0, 0};
    int option_index;
    int option;
    int opt_count = 0;
    char* dirPath = "./";

    while ((option = getopt_long(argc, argv, "ldfs", long_options, &option_index)) != -1) {
        switch (option) {
            case 'l':
                opt_count++;
                opts.show_links = 1;
                break;
            case 'd':
                opt_count++;
                opts.show_dirs = 1;
                break;
            case 'f':
                opt_count++;
                opts.show_files = 1;
                break;
            case 's':
                opt_count++;
                opts.sort = 1;
                break;
            default:
                printf("Usage: path [-l] [-d] [-f] [-s]\n");
                break;
        }
    }

    if (argc == 1 || (argc == 2 && opt_count > 0 )) {
        if(argc == 1){
            opts = (struct options){1, 1, 1, 1};
        }
        //printf("OPTIONS: %d\tARGC: %d\n", opt_count, argc);
        dirwalk(dirPath, &opts);
        return 0;
    }

    //printf("OPTIONS: %d\tARGC: %d\n", opt_count, argc);
    dirwalk(argv[optind], &opts);

    return 0;
}