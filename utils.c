#include "shell.h"
#include <stdio.h>
#include <unistd.h>

void print_prompt(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\033[1;34m%s\033[0m> ", cwd);
    } else {
        printf("> ");
    }
    fflush(stdout);
}
