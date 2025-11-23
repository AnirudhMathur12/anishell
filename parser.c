#include "shell.h"

char **get_args(char *line) {
    int capacity = 16;
    char **args = malloc(capacity * sizeof(char *));
    if (!args)
        return NULL;
    int count = 0;
    char *ptr = line;

    while (*ptr) {
        while (*ptr && isspace((unsigned char)*ptr))
            ptr++;

        if (*ptr == '\0')
            break;

        if (*ptr == '"') {
            ptr++;
            char *start = ptr;
            while (*ptr && *ptr != '"')
                ptr++;
            size_t len = (size_t)(ptr - start);
            char *token = strndup(start, len);
            if (!token)
                token = strdup("");
            args[count++] = token;
            if (*ptr == '"')
                ptr++;
        } else {
            char *start = ptr;
            while (*ptr && !isspace((unsigned char)*ptr))
                ptr++;
            size_t len = (size_t)(ptr - start);
            char *token = strndup(start, len);
            if (!token)
                token = strdup("");
            args[count++] = token;
        }
        if (count >= capacity - 1) {
            capacity *= 2;
            char **tmp = realloc(args, capacity * sizeof(char *));
            if (!tmp)
                break;
            args = tmp;
        }
    }
    args[count] = NULL;
    return args;
}

void free_args(char **args) {
    if (!args)
        return;
    for (int i = 0; args[i]; i++) {
        free(args[i]);
    }
    free(args);
}
