#ifndef SHELL_H
#define SHELL_H

#include <ctype.h>
#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

// Structs
typedef struct {
    char **items;
    int count;
} MatchList;

// Function Declarations

// input.c
void enableRawMode();
void disableRawMode();
void init_history();
char *read_input(void);
void add_to_history(char *line);

// parser.c
char **get_args(char *line);

// autocomplete.c
MatchList get_matching_files(const char *prefix);
void free_match_list(MatchList *list);

#endif
