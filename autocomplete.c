#include "shell.h"

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
    DIR *d;
    struct dirent *dir;

    d = opendir(".");
    if (d) {
        int capacity = 10;
        list.items = malloc(capacity * sizeof(char *));
        int prefix_len = strlen(prefix);

        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, prefix, prefix_len) == 0) {
                if (strcmp(dir->d_name, ".") == 0 ||
                    strcmp(dir->d_name, "..") == 0)
                    continue;

                if (list.count >= capacity) {
                    capacity *= 2;
                    list.items = realloc(list.items, capacity * sizeof(char *));
                }
                list.items[list.count++] = strdup(dir->d_name);
            }
        }
        closedir(d);
    }
    return list;
}
