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
    raw.c_lflag &= ~(ECHO | ICANON | ECHONL | IEXTEN);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char *read_input(void) {
    char c;
    char *buf = malloc(1024);
    int buf_len = 0;
    int cursor_pos = 0;
    int prev_iter = iter;

    memset(buf, 0, 1024);

    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == '\x1b') {
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) != 1)
                return NULL;
            if (read(STDIN_FILENO, &seq[1], 1) != 1)
                return NULL;

            if (seq[0] == '[') {
                switch (seq[1]) {
                case 'A': // UP ARROW (History)
                    if (prev_iter > 0) {
                        while (cursor_pos > 0) {
                            printf("\b");
                            cursor_pos--;
                        }
                        for (int i = 0; i < buf_len; i++)
                            printf(" ");
                        for (int i = 0; i < buf_len; i++)
                            printf("\b");

                        prev_iter--;
                        char *history_str = prev[prev_iter];
                        memset(buf, 0, 1024);
                        if (history_str)
                            strncpy(buf, history_str, 1023);

                        buf_len = strlen(buf);
                        cursor_pos = buf_len;
                        printf("%s", buf);
                    }
                    break;
                case 'B': // DOWN ARROW
                    if (prev_iter < iter) {
                        // Erase visual line
                        while (cursor_pos > 0) {
                            printf("\b");
                            cursor_pos--;
                        }
                        for (int i = 0; i < buf_len; i++)
                            printf(" ");
                        for (int i = 0; i < buf_len; i++)
                            printf("\b");

                        prev_iter++;
                        memset(buf, 0, 1024);
                        if (prev_iter < iter) {
                            strncpy(buf, prev[prev_iter], 1023);
                        }

                        buf_len = strlen(buf);
                        cursor_pos = buf_len;
                        printf("%s", buf);
                    }
                    break;

                case 'C': // RIGHT ARROW
                    if (cursor_pos < buf_len) {
                        printf("\033[C");
                        cursor_pos++;
                    }
                    break;
                case 'D': // LEFT ARROW
                    if (cursor_pos > 0) {
                        printf("\033[D");
                        cursor_pos--;
                    }
                    break;
                }
            }
            fflush(stdout);
            continue;
        }

        if (iscntrl(c)) {
            if (c == 10 || c == 13) { // ENTER
                buf[buf_len] = '\0';
                printf("\r\n");
                return buf;
            } else if (c == 127) { // BACKSPACE
                if (cursor_pos > 0) {
                    if (cursor_pos == buf_len) {
                        cursor_pos--;
                        buf_len--;
                        buf[buf_len] = '\0';
                        printf("\b \b");
                    } else {
                        memmove(&buf[cursor_pos - 1], &buf[cursor_pos],
                                buf_len - cursor_pos);
                        cursor_pos--;
                        buf_len--;
                        buf[buf_len] = '\0';

                        printf("\b");
                        printf("%s ", &buf[cursor_pos]);
                        for (int i = 0; i < (buf_len - cursor_pos + 1); i++)
                            printf("\b");
                    }
                }
            }
        } else if (buf_len < 1023) {

            if (cursor_pos == buf_len) {
                buf[cursor_pos] = c;
                cursor_pos++;
                buf_len++;
                printf("%c", c);
            } else {
                memmove(&buf[cursor_pos + 1], &buf[cursor_pos],
                        buf_len - cursor_pos);

                buf[cursor_pos] = c;
                buf_len++;
                cursor_pos++;
                printf("%s", &buf[cursor_pos - 1]);

                for (int i = 0; i < (buf_len - cursor_pos); i++) {
                    printf("\b");
                }
            }
        }
        fflush(stdout);
    }
    return buf;
}

void handle_sigint(int sig) { printf("\r\n"); }

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
    prev = malloc(sizeof(char *) * 1024);
    iter = 0;

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

        // ctrl D or empty input
        if (!buf || strlen(buf) == 0) {
            if (buf)
                free(buf);
            continue;
        }

        prev[iter++] = strdup(buf);
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
                    printf("lsh: No such file or directory\r\n");
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
                printf("lsh: command not found\n");
            }
            exit(EXIT_FAILURE);
        } else {
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
