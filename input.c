#include "shell.h"
#include <stdio.h>

struct termios orig_termios;
char **prev = NULL; // Command history buffer
int iter = 0;       // Command history iterator

// Tab completion vars
MatchList matches = {NULL, 0};
int match_index = 0;
int is_tabbing = 0;
char *original_prefix = NULL;
int last_match_len = 0;

void disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ECHONL | IEXTEN);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void init_history() {
    prev = malloc(sizeof(char *) * 1024);
    iter = 0;
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

        if (c != '\t' && is_tabbing) {
            is_tabbing = 0;
            free_match_list(&matches);
            if (original_prefix) {
                free(original_prefix);
                original_prefix = NULL;
            }
            last_match_len = 0;
        }

        if (c == '\t') {
            if (!is_tabbing) {
                int start = cursor_pos;

                while (start > 0 && buf[start - 1] != ' ') {
                    start--;
                }

                int len = cursor_pos - start;
                if (len >= 0) {
                    original_prefix = malloc(len + 1);
                    strncpy(original_prefix, &buf[start], len);
                    original_prefix[len] = '\0';

                    matches = get_matching_files(original_prefix);
                    if (matches.count > 0) {
                        is_tabbing = 1;
                        match_index = 0;
                        last_match_len = len;
                    } else {
                        free_match_list(&matches);
                    }
                }
            }

            if (is_tabbing && matches.count > 0) {
                char *current_match = matches.items[match_index];
                int start = buf_len;
                while (start > 0 && buf[start - 1] != ' ') {
                    start--;
                }

                int chars_to_back = buf_len - start;
                for (int i = 0; i < chars_to_back; i++) {
                    printf("\b");
                }
                printf("\033[K");

                buf_len = start;

                strcpy(&buf[buf_len], current_match);
                int match_len = strlen(current_match);
                buf_len += match_len;
                cursor_pos = buf_len;

                printf("%s", current_match);

                match_index = (match_index + 1) % matches.count;
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
            } else if (c == 4) { // Ctrl + D
                exit(0);
            } else if (c == 12) {
                printf("\033[2J\033[H");

                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("\033[1;34m%s\033[0m> ", cwd);
                } else {
                    printf("> ");
                }

                fflush(stdout);

                printf("%s", buf);
                fflush(stdout);

                continue;
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

void add_to_history(char *line) {
    if (prev && line) {
        prev[iter++] = strdup(line);
    }
}

void free_history_and_matches(void) {
    if (prev) {
        for (int i = 0; i < iter; i++) {
            free(prev[i]);
            prev[i] = NULL;
        }
        free(prev);
        prev = NULL;
        iter = 0;
    }
    if (original_prefix) {
        free(original_prefix);
        original_prefix = NULL;
    }
    free_match_list(&matches);
}
