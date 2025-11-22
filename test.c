#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

char **prev;
int iter;

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
                buf_iter--;
                printf("\b \b");
            }
        } else if (buf_iter < 1023) {
            buf[buf_iter++] = c;
            printf("%c", c);
        }

        fflush(stdout);
    }
}

int main() {
    enableRawMode();

    FILE *fp = fopen("inputs.txt", "w");
    char *input = "";
    prev = malloc(sizeof(char *) * 1024);
    iter = 0;
    while (strcmp(input, "exit") != 0) {
        fprintf(fp, "%s\n", input);
        input = read_input();
        prev[iter++] = strdup(input);
    }

    return 0;
}
