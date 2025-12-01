#include "shell.h"
#include <math.h>
#include <stdio.h>

void append_char(char **buffer, int *size, int *capacity, char c) {
    if (*size + 1 >= *capacity) {
        *capacity *= 2;
        char *temp = realloc(*buffer, *capacity * sizeof(char));
        if (!temp) {
            fprintf(stderr, "Allocation failed\n");
            exit(1);
        }
        *buffer = temp;
    }

    (*buffer)[*size] = c;
    (*size)++;
    (*buffer)[*size] = '\0';
}

void push_token(char ***args, int *count, int *capacity, char *tok) {
    if (*count + 1 >= *capacity) {
        *capacity *= 2;
        char **temp = realloc(*args, *capacity * sizeof(char *));
        if (!temp) {
            fprintf(stderr, "Allocation failed\n");
            exit(1);
        }
        *args = temp;
    }

    (*args)[*count] = strdup(tok);
    (*count)++;
}

char **get_args(char *line) {
    int args_cap = 16;
    int args_count = 0;
    char **args = malloc(args_cap * sizeof(char *));

    int buf_cap = 64;
    int buf_size = 0;
    char *buf = malloc(buf_cap);

    int in_double = 0;
    int in_single = 0;
    int escaped = 0;

    char *ptr = line;

    while (*ptr) {
        char c = *ptr;

        if (escaped) {
            append_char(&buf, &buf_size, &buf_cap, c);
            escaped = 0;
        } else if (c == '\\' && !in_single) {
            escaped = 1;
        } else if (c == '\'' && !in_double) {
            in_single = !in_single;
        } else if (c == '"' && !in_single) {
            in_double = !in_double;
        } else if (c == '#' && !in_single && !in_double) {
            if (buf_size == 0)
                break;
            else
                append_char(&buf, &buf_size, &buf_cap, c);
        } else if (isspace((unsigned char)c) && !in_double && !in_single) {
            if (buf_size > 0) {
                push_token(&args, &args_count, &args_cap, buf);
                buf_size = 0;
                buf[0] = '\0';
            }
        } else {
            append_char(&buf, &buf_size, &buf_cap, c);
        }

        ptr++;
    }

    if (buf_size > 0) {
        push_token(&args, &args_count, &args_cap, buf);
    }

    args[args_count] = NULL;
    free(buf);

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
