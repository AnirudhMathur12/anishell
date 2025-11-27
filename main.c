#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

FILE *history;

void handle_sigint(int sig) { printf("\r\n"); }

int is_builtin(char *cmd) {
    return (strcmp(cmd, "cd") == 0 || strcmp(cmd, "exit") == 0 ||
            strcmp(cmd, "export") == 0 || strcmp(cmd, "alias") == 0);
}

void handle_builtin(char **args) {
    if (strcmp(args[0], "export") == 0) {
        if (args[1]) {
            char *eq = strchr(args[1], '=');
            if (eq) {
                *eq = '\0';
                set_shell_var(args[1], eq + 1, 1);
            }
        }
    } else if (strcmp(args[0], "alias") == 0) {
        if (args[1] && args[2]) {
            add_alias(args[1], args[2]);
        }
    } else if (strcmp(args[0], "cd") == 0) {
        char *path = args[1] ? args[1] : get_shell_var("HOME");
        if (path && chdir(path) != 0) {
            perror("cd");
        }
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }
}

int setup_redirections(char **args) {
    int first_redir = -1;

    for (int i = 0; args[i]; i++) {
        int fd = -1;
        int is_redir = 0;

        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Syntax error: expected file after >\n");
                return -1;
            }

            fd = open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
            if (fd == -1) {
                perror("open");
                return -1;
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);
            is_redir = 1;
        } else if (strcmp(args[i], ">>") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Syntax error: expected file after >>\n");
                return -1;
            }

            fd = open(args[i + 1], O_WRONLY | O_APPEND | O_CREAT, 0644);
            if (fd == -1) {
                perror("open");
                return -1;
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);
            is_redir = 1;
        } else if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Syntax error: expected file after <\n");
                return -1;
            }

            fd = open(args[i + 1], O_RDONLY);
            if (fd == -1) {
                perror("open");
                return -1;
            }

            dup2(fd, STDIN_FILENO);
            close(fd);
            is_redir = 1;
        } else if (strcmp(args[i], "2>") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Syntax error: expected file after 2>\n");
                return -1;
            }

            fd = open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
            if (fd == -1) {
                perror("open");
                return -1;
            }

            dup2(fd, STDERR_FILENO);
            close(fd);
            is_redir = 1;
        } else if (strcmp(args[i], "&>") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Syntax error: expected file after &>\n");
                return -1;
            }
            fd = open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
            if (fd == -1) {
                perror("open");
                return -1;
            }
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            is_redir = 1;
        } else if (strcmp(args[i], "2>&1") == 0) {
            dup2(STDOUT_FILENO, STDERR_FILENO);
            is_redir = 1;
        }

        if (is_redir && first_redir == -1) {
            first_redir = i;
        }
    }

    if (first_redir != -1) {
        args[first_redir] = NULL;
    }

    return 0;
}

void execute_pipeline(char **args) {
    int num_cmds = 0;
    char *commands[64][64];

    int arg_idx = 0;
    int cmd_idx = 0;
    commands[num_cmds][0] = NULL;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            commands[num_cmds][cmd_idx] = NULL;
            num_cmds++;
            cmd_idx = 0;
        } else {
            commands[num_cmds][cmd_idx++] = args[i];
        }
    }
    commands[num_cmds][cmd_idx] = NULL;
    num_cmds++;

    int i;
    int prev_pipe_read = -1;
    int fd[2];

    for (i = 0; i < num_cmds; i++) {
        if (i < num_cmds - 1) {
            if (pipe(fd) == -1) {
                perror("pipe");
                return;
            }
        }

        pid_t pid = fork();

        if (pid == 0) {

            disableRawMode();
            signal(SIGINT, SIG_DFL);

            if (i > 0 && prev_pipe_read != -1) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }

            if (i < num_cmds - 1) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                close(fd[0]);
            }

            if (setup_redirections(commands[i]) == -1) {
                exit(1);
            }

            if (strcmp(commands[i][0], "echo") == 0) {
                for (int j = 1; commands[i][j]; j++) {
                    printf("%s", commands[i][j]);
                    if (commands[i][j + 1])
                        printf(" ");
                }
                printf("\n");
                exit(0);
            }

            if (execvp(commands[i][0], commands[i]) == -1) {
                fprintf(stderr, "anishell: command not found: %s\n",
                        commands[i][0]);
                exit(127);
            }
        }

        if (i > 0) {
            close(prev_pipe_read);
        }
        if (i < num_cmds - 1) {
            close(fd[1]);
            prev_pipe_read = fd[0];
        }
    }

    for (i = 0; i < num_cmds; i++) {
        wait(NULL);
    }

    enableRawMode();
}

void exec_command_line(char *line) {
    char **args = get_args(line);
    if (!args || !args[0]) {
        free_args(args);
        return;
    }

    resolve_aliases(&args);
    expand_args(args);

    int has_pipe = 0;
    for (int i = 0; args[i]; i++) {
        if (strcmp(args[i], "|") == 0)
            has_pipe = 1;
    }

    if (!has_pipe && is_builtin(args[0])) {
        handle_builtin(args);
    } else {
        execute_pipeline(args);
    }

    free_args(args);
}

void load_rc_file(void) {
    char path[1024];
    char *home = getenv("HOME");
    if (!home)
        return;

    snprintf(path, sizeof(path), "%s/.anishellrc", home);
    FILE *fp = fopen(path, "r");
    if (!fp)
        return;

    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) != -1) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) > 0)
            exec_command_line(line);
    }
    fclose(fp);
    if (line)
        free(line);
}

int main(void) {
    enableRawMode();
    atexit(disableRawMode);
    init_history();

    char history_path[1024];
    char *home = getenv("HOME");
    if (home) {
        snprintf(history_path, sizeof(history_path), "%s/.anishell_history",
                 home);
        load_history_from_file(history_path);
    }

    init_config();

    atexit(free_config);
    atexit(free_history_and_matches);
    atexit(save_history_to_file);

    load_rc_file();

    char *buf = NULL;
    char cwd[1024];

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

        exec_command_line(buf);

        free(buf);
    } while (1);
    return 0;
}
