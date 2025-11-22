#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

char **prev;
int iter;

struct termios orig_termios;

void disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char *read_input(void) {

    char c;

    char *buf = malloc(1024);
    char *stored_temp;
    int stored_size = 0;
    int buf_iter = 0;
    int prev_iter = iter;
    int stored = 0;

    memset(buf, 0, 1024);

    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
        if (c == '\x1b') {
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) != 1)
                return 0;
            if (read(STDIN_FILENO, &seq[1], 1) != 1)
                return 0;

            if (seq[0] == '[') {
                switch (seq[1]) {
                case 'A':

                    if (prev_iter > 0) {
                        if (!stored) {
                            stored_temp = buf;
                            stored_size = buf_iter;
                            stored = 1;
                        }
                        for (int i = 0; i < buf_iter; i++) {
                            printf("\b \b");
                        }
                        buf = prev[--prev_iter];
                        buf_iter = strlen(buf);
                        printf("%s", buf);
                    } else {
                        printf("\a");
                    }
                    break;
                case 'B':
                    if (prev_iter < iter) {
                        for (int i = 0; i < buf_iter; i++) {
                            printf("\b \b");
                        }
                        buf = prev[prev_iter++];
                        buf_iter = strlen(buf);
                        printf("%s", buf);
                    } else {
                        if (stored) {
                            for (int i = 0; i < buf_iter; i++) {
                                printf("\b \b");
                            }
                            buf = stored_temp;
                            buf_iter = stored_size;
                            stored = 0;
                            printf("%s", buf);
                        }
                        printf("\a");
                    }
                    break;
                case 'C':
                    printf("RIGHT ARROW DETECTED!\r\n");
                    break;
                case 'D':
                    printf("LEFT ARROW DETECTED!\r\n");
                    break;
                }
            }

            fflush(stdout);
            continue;
        }

        if (iscntrl(c)) {
            if (c == 10 || c == 13) {
                buf[buf_iter] = '\0';
                printf("\r\n");
                return buf;
            } else if (c == 127) {
                if (buf_iter > 0) {
                    buf_iter--;
                    buf[buf_iter] = '\0';
                    printf("\b \b");
                }
            }
        } else if (buf_iter < 1023) {
            buf[buf_iter++] = c;
            printf("%c", c);
        }

        fflush(stdout);
    }
}

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

    enableRawMode();
    char *buf = NULL;
    char cwd[1024];
    size_t len = 0;
    prev = malloc(sizeof(char *) * 1024);
    iter = 0;

    char **args = NULL;
    signal(SIGINT, handle_sigint);

    do {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);
        } else {
            perror("getcwd() error");
            printf("> ");
        }
        fflush(stdout);

        // if (getline(&buf, &len, stdin) == -1) {
        //     if (feof(stdin)) {
        //         printf("\n");
        //         break;
        //     } else {
        //         clearerr(stdin);
        //         continue;
        //     }
        // }

        buf = read_input();
        prev[iter++] = strdup(buf);

        if (strcmp(buf, "\n") == 0) {
            continue;
        }

        args = get_args(buf);

        if (args[0] == NULL) {
            free(args);
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
                if (home == NULL) {
                    fprintf(stderr, "lsh: could not find HOME variable");
                    fflush(stdout);
                } else {
                    if (chdir(home) != 0) {
                        perror("lsh");
                    }
                    fflush(stdout);
                }
            } else {
                if (chdir(args[1]) != 0) {
                    perror("lsh");
                    fflush(stdout);
                }
                free(args);
                continue;
            }
        } else if (strcmp(args[0], "exit") == 0) {
            exit(0);
        }

        int pid = fork();
        int status;

        if (pid == 0) {
            // Child
            //
            i = 0;
            while (args[i]) {
                if (strcmp(">", args[i]) == 0) {
                    if (args[i + 1] == NULL) {
                        fprintf(stderr, "lsh: Expected output file\n");
                        fflush(stdout);
                        exit(EXIT_FAILURE);
                    }
                    const char *out = args[i + 1];

                    int fd = open(out, O_WRONLY | O_TRUNC | O_CREAT, 0644);

                    if (fd == -1) {
                        perror("open");
                        fflush(stdout);
                        exit(EXIT_FAILURE);
                    }

                    if (dup2(fd, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        fflush(stdout);
                        exit(EXIT_FAILURE);
                    }

                    close(fd);

                    args[i] = NULL;

                    break;
                } else if (strcmp(">>", args[i]) == 0) {
                    if (args[i + 1] == NULL) {
                        fprintf(stderr, "lsh: Expected output file\n");
                        fflush(stdout);
                        exit(EXIT_FAILURE);
                    }
                    const char *out = args[i + 1];

                    int fd = open(out, O_WRONLY | O_APPEND | O_CREAT, 0644);

                    if (fd == -1) {
                        perror("open");
                        fflush(stdout);
                        exit(EXIT_FAILURE);
                    }

                    if (dup2(fd, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        fflush(stdout);
                        exit(EXIT_FAILURE);
                    }

                    close(fd);

                    args[i] = NULL;

                    break;
                }

                i++;
            }

            if (execvp(args[0], args) == -1) {
                perror("lsh");
                fflush(stdout);
            }

            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("lsh\n");
            fflush(stdout);
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
