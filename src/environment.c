/**
 * @file environment.c
 */
#include "spm.h"

/**
 *
 * @param env
 */
void runtime_export(char **env) {
    char *borne[] = {
            "bash",
            "dash",
            "zsh",
            NULL,
    };
    char *unborne[] = {
            "csh"
            "tcsh",
            NULL,
    };

    char output[BUFSIZ];
    char export_command[7]; // export=6 and setenv=6... convenient
    char *_sh = getenv("SHELL");
    char *sh = basename(_sh);
    if (sh == NULL) {
        fprintf(stderr, "echo SHELL environment variable is not defined");
        exit(1);
    }

    for (size_t i = 0; borne[i] != NULL; i++) {
        if (strcmp(sh, borne[i]) == 0) {
            strcpy(export_command, "export");
            break;
        }
    }
    for (size_t i = 0; unborne[i] != NULL; i++) {
        if (strcmp(sh, unborne[i]) == 0) {
            strcpy(export_command, "setenv");
            break;
        }
    }

    for (size_t i = 0; env[i] != NULL; i++) {
        char **pair = split(env[i], "=");
        sprintf(output, "%s %s=\"%s\"", export_command, pair[0], pair[1] ? pair[1] : "");
        puts(output);
        split_free(pair);
    }
}

/**
 *
 * @param env
 * @return
 */
char **runtime_copy(char **env) {
    char **envp = NULL;
    size_t env_count;
    for (env_count = 0; env[env_count] != NULL; env_count++);

    envp = (char **)calloc(env_count + 1, sizeof(char *));
    for (size_t i = 0; i < env_count; i++) {
        size_t len = strlen(env[i]);
        envp[i] = (char *)calloc(len + 1, sizeof(char));
        memcpy(envp[i], env[i], len);
    }
    return envp;
}

void runtime_free(char **env) {
    for (size_t i = 0; env[i] != NULL; i++) {
        free(env[i]);
    }
    free(env);
}