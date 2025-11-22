#include "shell.h"
#include <string.h>

#define MAX_VARS 4096
#define MAX_ALIASES 4096

ShellVar env_vars[MAX_VARS];
int env_count = 0;

Alias alias_table[MAX_ALIASES];
int alias_count = 0;

void init_config() {
    env_count = 0;
    alias_count = 0;
}

void set_shell_var(char *key, char *value, int exported) {
    // update existing var
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].key, key) == 0) {
            free(env_vars[i].value);
            env_vars[i].value = strdup(value);

            if (exported) {
                env_vars[i].exported = 1;
                setenv(key, value, 1);
            }
            return;
        }
    }

    // create new one
    if (env_count < MAX_VARS) {
        env_vars[env_count].key = strdup(key);
        env_vars[env_count].value = strdup(value);
        env_vars[env_count].exported = exported;
        if (exported)
            setenv(key, value, 1);
        env_count++;
    }
}

char *get_shell_var(char *key) {
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].key, key) == 0) {
            return env_vars[i].value;
        }
    }

    return getenv(key);
}

void add_alias(char *name, char *value) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(alias_table[i].name, name) == 0) {
            free(alias_table[i].value);
            alias_table[i].value = strdup(value);
            return;
        }
    }
    if (alias_count < MAX_ALIASES) {
        alias_table[alias_count].name = strdup(name);
        alias_table[alias_count].value = strdup(value);
        alias_count++;
    }
}

char *get_alias(char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(alias_table[i].name, name) == 0) {
            return alias_table[i].value;
        }
    }
    return NULL;
}

int resolve_aliases(char ***args_ptr) {
    char **args = *args_ptr;
    if (!args[0])
        return 0;

    char *alias_val = get_alias(args[0]);
    if (!alias_val)
        return 0;

    char *val_copy = strdup(alias_val);
    char **alias_tokens = get_args(val_copy);

    int alias_len = 0;
    while (alias_tokens[alias_len])
        alias_len++;
    int args_len = 0;
    while (args[args_len])
        args_len++;

    char **new_args = malloc((alias_len + args_len) * sizeof(char *));

    int k = 0;
    for (int i = 0; i < alias_len; i++)
        new_args[k++] = strdup(alias_tokens[i]);
    for (int i = 1; i < args_len; i++)
        new_args[k++] = args[i];
    new_args[k] = NULL;

    free(args);
    free(alias_tokens);

    *args_ptr = new_args;
    return 1;
}

void expand_args(char **args) {
    int i = 0;
    while (args[i]) {
        if (args[i][0] == '~') {
            char *home = get_shell_var("HOME");
            if (home) {
                size_t new_size = strlen(home) + strlen(args[i]) + 1;
                char *new_arg = malloc(new_size);
                snprintf(new_arg, new_size, "%s%s", home, &args[i][1]);
                args[i] = new_arg;
            }
        } else if (args[i][0] == '$') {
            char *key = &args[i][1];
            char *val = get_shell_var(key);
            if (val) {
                args[i] = strdup(val);
            } else {
                args[i] = strdup("");
            }
        }
        i++;
    }
}
