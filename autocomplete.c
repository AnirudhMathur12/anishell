#include "shell.h"
#if defined(__APPLE__) && defined(__MACH__)
#include <sys/dirent.h>
#elif (__linux__)
#include <dirent.h>
#endif
#include <sys/stat.h>

void free_match_list(MatchList *list) {
    if (list->items) {
        for (int i = 0; i < list->count; i++) {
            free(list->items[i]);
        }
        free(list->items);
        list->items = NULL;
    }
    list->count = 0;
}

MatchList get_matching_files(const char *prefix) {
    MatchList list = {NULL, 0};
    char dir_path[1024] = ".";
    char file_prefix[1024] = "";
    char full_path[2048];

    char *last_slash = strrchr(prefix, '/');
    if (last_slash) {
        size_t dir_len = last_slash - prefix + 1;
        strncpy(dir_path, prefix, dir_len);
        dir_path[dir_len] = '\0';
        strcpy(file_prefix, last_slash + 1);
    } else {
        strcpy(file_prefix, prefix);
    }

    DIR *d = opendir(dir_path);
    if (d) {
        int capacity = 10;
        list.items = malloc(capacity * sizeof(char *));
        int prefix_len = strlen(file_prefix);
        struct dirent *dir;

        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, file_prefix, prefix_len) == 0) {
                if (strcmp(dir->d_name, ".") == 0 ||
                    strcmp(dir->d_name, "..") == 0)
                    continue;

                if (list.count >= capacity) {
                    int new_capacity = capacity * 2;
                    char **tmp =
                        realloc(list.items, new_capacity * sizeof(char *));
                    if (!tmp) {
                        free_match_list(&list);
                        closedir(d);
                        return list;
                    }
                    list.items = tmp;
                    capacity = new_capacity;
                }

                if (strcmp(dir_path, ".") == 0) {
                    snprintf(full_path, sizeof(full_path), "%s", dir->d_name);
                } else {
                    snprintf(full_path, sizeof(full_path), "%s%s", dir_path,
                             dir->d_name);
                }

                struct stat st;

                char stat_path[2048];
                snprintf(stat_path, sizeof(stat_path), "%s%s", dir_path,
                         dir->d_name);

                if (stat(stat_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                    strcat(full_path, "/");
                }

                list.items[list.count++] = strdup(full_path);
            }
        }

        closedir(d);
    }

    return list;
}
