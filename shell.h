#ifndef SHELL_H
#define SHELL_H

#define _GNU_SOURCE

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
void enableRawMode(void);
void disableRawMode(void);
void init_history(void);
char *read_input(void);
void load_history_from_file(const char *path);
void save_history_to_file(void);
void add_to_history(char *line);
void free_history_and_matches(void);
void print_history(void);

// parser.c
char **get_args(char *line);
void free_args(char **args);

// autocomplete.c
MatchList get_matching_files(const char *prefix);
void free_match_list(MatchList *list);

// config.c
void set_last_exit_status(uint8_t status);
uint8_t get_last_exit_status(void);
void init_config(void);
void set_shell_var(char *key, char *value, int exported);
char *get_shell_var(char *key);
void add_alias(char *name, char *value);
char *get_alias(char *name);
int resolve_aliases(char ***args_ptr);
void expand_args(char **args);
void free_config(void);

// utils.c
void print_prompt(void);

#endif
