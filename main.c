#include "shell.h"

void handle_sigint(int sig) { printf("\r\n"); }

void exec_command_line(char *line) {
    char **args = get_args(line);
    if (!args[0]) {
        free(args);
        return;
    }

    resolve_aliases(&args);

    expand_args(args);

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
    } else {
        int pid = fork();
        if (pid == 0) {
            disableRawMode();
            signal(SIGINT, SIG_DFL);

            int i = 0;
            while (args[i]) {
                if (strcmp(">", args[i]) == 0) {
                    int fd =
                        open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    args[i] = NULL;
                    break;
                }
                i++;
            }

            if (execvp(args[0], args) == -1) {
                printf("anishell: command not found: %s\n", args[0]);
            }
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
            enableRawMode();
        }
    }

    free(args);
}

void load_rc_file() {
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
    init_history();
    init_config();

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
