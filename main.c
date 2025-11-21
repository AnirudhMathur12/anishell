#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void handle_sigint(int sig) { printf("\n"); }

char **get_args(char *line) {
    int size = 8;
    char **args = malloc(size * sizeof(char *));
    int i = 0;
    char *tok = strtok(line, " \t\n");

    while (tok) {
        args[i++] = tok;
        tok = strtok(NULL, " \t\n");

        if (i == size - 1) {
            args = realloc(args, size * sizeof(char *) * 1.5);
        }
    }

    args[i] = NULL;

    return args;
}

int main(void) {

    char *buf = NULL;
    char cwd[1024];
    size_t len = 0;

    char **args = NULL;
    signal(SIGINT, handle_sigint);

    do {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);
        } else {
            perror("getcwd() error");
            printf("> ");
        }

        if (getline(&buf, &len, stdin) == -1) {
            if (feof(stdin)) {
                printf("\n");
                break;
            } else {
                clearerr(stdin);
                continue;
            }
        }

        args = get_args(buf);

        int i = 0;
        while (args[i]) {
            if (args[i][0] == '~') {
                char *home = getenv("HOME");
                if (home) {
                    size_t new_size = strlen(home) + strlen(args[i]) + 1;
                    char *new_arg = malloc(new_size);
                    snprintf(new_arg, new_size, "%s%s", home, &args[i][1]);
                    args[i] = new_arg;
                }
            }
            i++;
        }

        if (strcmp(args[0], "cd") == 0) {
            chdir(args[1]);
            continue;
        } else if (strcmp(args[0], "exit") == 0) {
            exit(0);
        }

        int pid = fork();
        int status;

        if (pid == 0) {
            // Child
            if (execvp(args[0], args) == -1) {
                perror("lsh");
            }

            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("lsh\n");
        } else {
            // Parent
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }

        free(args);

    } while (1);
    return 0;
}
