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

typedef struct {
    char *key;
    char *value;
    int exported;
} ShellVar;

typedef struct {
    char *name;
    char *value;
} Alias;

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

// config.c
void init_config();
void set_shell_var(char *key, char *value, int exported);
char *get_shell_var(char *key);
void add_alias(char *name, char *value);
char *get_alias(char *name);
int resolve_aliases(char **args_ptr);
void expand_args(char **args);

#endif
