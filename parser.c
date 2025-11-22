#include "shell.h"

char **get_args(char *line) {
    int capacity = 16;
    char **args = malloc(capacity * sizeof(char *));
    int count = 0;
    char *ptr = line;

    while (*ptr) {
        while (*ptr && isspace(*ptr))
            ptr++;

        if (*ptr == '\0')
            break;

        if (*ptr == '"') {
            ptr++;
            args[count++] = ptr;
            while (*ptr && *ptr != '"')
                ptr++;
            if (*ptr == '"') {
                *ptr = '\0';
                ptr++;
            }
        } else {
            args[count++] = ptr;
            while (*ptr && !isspace(*ptr))
                ptr++;
            if (*ptr && isspace(*ptr)) {
                *ptr = '\0';
                ptr++;
            }
        }
        if (count >= capacity - 1) {
            capacity *= 2;
            args = realloc(args, capacity * sizeof(char *));
        }
    }
    args[count] = NULL;
    return args;
}
