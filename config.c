#include "shell.h"
#include <stdint.h>
#include <string.h>

#define MAX_VARS 4096
#define MAX_ALIASES 4096

ShellVar env_vars[MAX_VARS];
int env_count = 0;

Alias alias_table[MAX_ALIASES];
int alias_count = 0;

static uint8_t last_exit_status;

void set_last_exit_status(uint8_t status) { last_exit_status = status; }

uint8_t get_last_exit_status(void) { return last_exit_status; }

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
    if (!args || !args[0])
        return 0;

    char *alias_val = get_alias(args[0]);
    if (!alias_val)
        return 0;

    char *val_copy = strdup(alias_val);
    if (!val_copy)
        return 0;

    char **alias_tokens = get_args(val_copy);
    if (!alias_tokens) {
        free(val_copy);
        return 0;
    }

    int alias_len = 0;
    while (alias_tokens[alias_len])
        alias_len++;
    int args_len = 0;
    while (args[args_len])
        args_len++;

    char **new_args = malloc((alias_len + args_len + 1) * sizeof(char *));
    if (!new_args) {
        free_args(alias_tokens);
        free(val_copy);
        return 0;
    }

    int k = 0;
    for (int i = 0; i < alias_len; i++)
        new_args[k++] = strdup(alias_tokens[i]); // duplicate alias tokens
    for (int i = 1; i < args_len; i++)
        new_args[k++] = strdup(args[i]); // duplicate remaining original args
    new_args[k] = NULL;

    // free old structures
    free_args(args);
    free_args(alias_tokens);
    free(val_copy);

    *args_ptr = new_args;
    return 1;
}

char *expand_token(char *token) {
    char *result = malloc(4096);
    char *res_ptr = result;
    char *ptr = token;

    while (*ptr) {
        if (*ptr == '$') {
            ptr++;
            char var_name[256];
            int v_index = 0;
            char status_st[16] = {0};

            if (*ptr == '?') {
                var_name[v_index++] = *ptr++;
            } else {
                while (*ptr && (isalnum((unsigned char)*ptr) || *ptr == '_')) {
                    var_name[v_index++] = *ptr++;
                }
            }

            var_name[v_index] = '\0';

            char *val = NULL;

            if (strcmp(var_name, "?") == 0) {
                snprintf(status_st, sizeof(status_st), "%d", last_exit_status);
                val = status_st;
            } else {
                val = get_shell_var(var_name);
            }

            if (val) {
                strcpy(res_ptr, val);
                res_ptr += strlen(val);
            }
        } else {
            *res_ptr++ = *ptr++;
        }
    }

    *res_ptr = '\0';
    return result;
}

void expand_args(char **args) {
    for (int i = 0; args[i]; i++) {
        if (args[i][0] == '~') {
            char *home = get_shell_var("HOME");
            if (home) {
                char *after_tilde = args[i] + 1;

                size_t len = strlen(home) + strlen(after_tilde) + 1;
                char *new_arg = malloc(len);

                if (new_arg) {
                    snprintf(new_arg, len, "%s%s", home, after_tilde);

                    free(args[i]);
                    args[i] = new_arg;
                }
            }
        }

        if (strchr(args[i], '$')) {
            char *expanded = expand_token(args[i]);
            if (expanded) {
                free(args[i]);
                args[i] = expanded;
            }
        }
    }
}

void free_config(void) {
    // free env vars
    for (int i = 0; i < env_count; i++) {
        free(env_vars[i].key);
        free(env_vars[i].value);
        env_vars[i].key = NULL;
        env_vars[i].value = NULL;
    }
    env_count = 0;

    // free aliases
    for (int i = 0; i < alias_count; i++) {
        free(alias_table[i].name);
        free(alias_table[i].value);
        alias_table[i].name = NULL;
        alias_table[i].value = NULL;
    }
    alias_count = 0;
}
