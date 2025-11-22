#include "shell.h"

void handle_sigint(int sig) { printf("\r\n"); }

int main(void) {
    enableRawMode();
    init_history();

    char *buf = NULL;
    char cwd[1024];
    char **args = NULL;

    signal(SIGINT, handle_sigint);

    do {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("\033[1;34m%s\033[0m> ", cwd);
        } else {
            printf("> ");
        }
        fflush(stdout);

        buf = read_input();

        if (!buf || strlen(buf) == 0) {
            if (buf)
                free(buf);
            continue;
        }

        add_to_history(buf);

        args = get_args(buf);

        if (args[0] == NULL) {
            free(args);
            free(buf);
            continue;
        }

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
            if (args[1] == NULL) {
                char *home = getenv("HOME");
                if (home)
                    chdir(home);
            } else {
                if (chdir(args[1]) != 0) {
                    printf("anishell: No such file or directory\r\n");
                }
            }
            free(args);
            free(buf);
            continue;
        } else if (strcmp(args[0], "exit") == 0) {
            free(args);
            free(buf);
            exit(0);
        }

        int pid = fork();
        int status;

        if (pid == 0) {
            // CHILD PROCESS

            disableRawMode();

            signal(SIGINT, SIG_DFL);

            i = 0;
            while (args[i]) {
                if (strcmp(">", args[i]) == 0) {
                    int fd =
                        open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    args[i] = NULL;

                    break;
                } else if (strcmp(">>", args[i]) == 0) {
                    int fd =
                        open(args[i + 1], O_WRONLY | O_APPEND | O_CREAT, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    args[i] = NULL;
                    break;
                }
                i++;
            }

            if (execvp(args[0], args) == -1) {
                printf("anishell: command not found\n");
            }
            exit(EXIT_FAILURE);
        } else {
            // PARENT PROCESS
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));

            enableRawMode();
        }

        free(args);
        free(buf);

    } while (1);
    return 0;
}
