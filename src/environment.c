/**
 * @file environment.c
 */
#include "spm.h"

/**
 * Print a shell-specific listing of environment variables to `stdout`
 *
 * Example:
 * ~~~{.c}
 * int main(int argc, char *argv[], char *arge[]) {
 *     RuntimeEnv *rt = runtime_copy(arge);
 *     runtime_export(rt, NULL);
 *     runtime_free(rt);
 *     return 0;
 * }
 * ~~~
 *
 * Usage:
 * ~~~{.sh}
 * $ gcc program.c
 * $ ./a.out
 * PATH="/thing/stuff/bin:/example/please/bin"
 * SHELL="/your/shell"
 * CC="/your/compiler"
 * ...=...
 *
 * # You can also use this to modify the shell environment
 * # (use `runtime_set` to manipulate the output)
 * $ source $(./a.out)
 * ~~~
 *
 * Example of exporting specific keys from the environment:
 *
 * ~~~{.c}
 * int main(int argc, char *argv[], char *arge[]) {
 *     RuntimeEnv *rt = runtime_copy(arge);
 *
 *     // inline declaration
 *     runtime_export(rt, (char *[]) {"PATH", "LS_COLORS", NULL});
 *
 *     // standard declaration
 *     char *keys_to_export[] = {
 *         "PATH", "LS_COLORS", NULL
 *     }
 *     runtime_export(rt, keys_to_export);
 *
 *     runtime_free(rt);
 *     return 0;
 * }
 * ~~~
 *
 * @param env `RuntimeEnv` structure
 * @param keys Array of keys to export. A value of `NULL` exports all environment keys
 */
void runtime_export(RuntimeEnv *env, char **keys) {
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

    for (size_t i = 0; i < strlist_count(env); i++) {
        char **pair = split(strlist_item(env, i), "=");
        if (keys != NULL) {
            for (size_t j = 0; keys[j] != NULL; j++) {
                if (strcmp(keys[j], pair[0]) == 0) {
                    sprintf(output, "%s %s=\"%s\"", export_command, pair[0], pair[1] ? pair[1] : "");
                    puts(output);
                }
            }
        }
        else {
            sprintf(output, "%s %s=\"%s\"", export_command, pair[0], pair[1] ? pair[1] : "");
            puts(output);
        }
        split_free(pair);
    }
}

/**
 * Populate a `RuntimeEnv` structure
 *
 * Example:
 *
 * ~~~{.c}
 * int main(int argc, char *argv[], char *arge[]) {
 *     RuntimeEnv *rt = NULL;
 *     // Example 1: Copy the shell environment
 *     rt = runtime_copy(arge);
 *     // Example 2: Create your own environment
 *     rt = runtime_copy((char *[]) {"SHELL=/bin/bash", "PATH=/opt/secure:/bin:/usr/bin"})
 *
 *     runtime_free(rt);
 *     return 0;
 * }
 * ~~~
 *
 * @param env Array of strings in `var=value` format
 * @return `RuntimeEnv` structure
 */
RuntimeEnv *runtime_copy(char **env) {
    RuntimeEnv *rt = NULL;
    size_t env_count;
    for (env_count = 0; env[env_count] != NULL; env_count++);

    rt = strlist_init();
    for (size_t i = 0; i < env_count; i++) {
        strlist_append(rt, env[i]);
    }
    return rt;
}

/**
 * Determine whether or not a key exists in the runtime environment
 *
 * Example:
 *
 * ~~~{.c}
 * int main(int argc, char *argv[], char *arge[]) {
 *     RuntimeEnv *rt = runtime_copy(arge);
 *     if (runtime_contains(rt, "PATH") {
 *         // $PATH is present
 *     }
 *     else {
 *         // $PATH is NOT present
 *     }
 *
 *     runtime_free(rt);
 *     return 0;
 * }
 * ~~~
 *
 * @param env `RuntimeEnv` structure
 * @param key Environment variable string
 * @return  -1=no, positive_value=yes
 */
ssize_t runtime_contains(RuntimeEnv *env, const char *key) {
    ssize_t result = -1;
    for (size_t i = 0; i < strlist_count(env); i++) {
        char **pair = split(strlist_item(env, i), "=");
        if (pair == NULL) {
            break;
        }
        if (strcmp(pair[0], key) == 0) {
            result = i;
            split_free(pair);
            break;
        }
        split_free(pair);
    }
    return result;
}

/**
 * Retrieve the value of a runtime environment variable
 *
 * Example:
 *
 * ~~~{.c}
 * int main(int argc, char *argv[], char *arge[]) {
 *     RuntimeEnv *rt = runtime_copy(arge);
 *     char *path = runtime_get("PATH");
 *     if (path == NULL) {
 *         // handle error
 *     }
 *
 *     runtime_free(rt);
 *     return 0;
 * }
 * ~~~
 *
 * @param env `RuntimeEnv` structure
 * @param key Environment variable string
 * @return success=string, failure=`NULL`
 */
char *runtime_get(RuntimeEnv *env, const char *key) {
    char *result = NULL;
    ssize_t key_offset = runtime_contains(env, key);
    if (key_offset != -1) {
        char **pair = split(strlist_item(env, key_offset), "=");
        result = join(&pair[1], "=");
        split_free(pair);
    }
    return result;
}

/**
 * Parse an input string and expand any environment variable(s) found
 *
 * Example:
 *
 * ~~~{.c}
 * int main(int argc, char *argv[], char *arge[]) {
 *     RuntimeEnv *rt = runtime_copy(arge);
 *     char *secure_path = runtime_expand_var(rt, "/opt/secure:$PATH:/aux/bin");
 *     if (secure_path == NULL) {
 *         // handle error
 *     }
 *     // secure_path = "/opt/secure:/your/original/path/here:/aux/bin");
 *
 *     runtime_free(rt);
 *     return 0;
 * }
 * ~~~
 *
 * @param env `RuntimeEnv` structure
 * @param input String to parse
 * @return success=expanded string, failure=`NULL`
 */
char *runtime_expand_var(RuntimeEnv *env, const char *input) {
    const char delim = '$';
    const char *delim_literal = "$$";
    const char *escape = "\\";
    char *expanded = calloc(BUFSIZ, sizeof(char));
    if (expanded == NULL) {
        perror("could not allocate runtime_expand_var buffer");
        fprintf(SYSERROR);
        return NULL;
    }

    // If there's no environment variables to process return a copy of the input string
    if (strchr(input, delim) == NULL) {
        return strdup(input);
    }

    // Parse the input string
    size_t i;
    for (i = 0; i < strlen(input); i++) {
        char var[MAXNAMLEN];    // environment variable name
        memset(var, '\0', MAXNAMLEN);   // zero out name

        // Ignore closing brace
        if (input[i] == '}') {
            i++;
        }

        // Handle literal statement "$$var"
        // Value becomes "\$var"
        if (strncmp(&input[i], delim_literal, strlen(delim_literal)) == 0) {
            strncat(expanded, escape, strlen(escape));
            strncat(expanded, &delim, 1);
            i += strlen(delim_literal);
            // Ignore opening brace
            if (input[i] == '{') i++;
        }

        // Handle variable when encountering a single $
        // Value expands from "$var" to "environment value of var"
        if (input[i] == delim) {
            // Ignore opening brace
            if (input[i] == '{') i++;
            char *tmp = NULL;
            i++;

            // Construct environment variable name from input
            // "$ var" == no
            // "$-*)!@ == no
            // "$var" == yes
            for (size_t c = 0; isalnum(input[i]) || input[i] == '_'; c++, i++) {
                var[c] = input[i];
            }

            tmp = runtime_get(env, var);
            if (tmp == NULL) {
                // This mimics shell behavior in general.
                // Prevent appending whitespace when an environment variable does not exist
                if (i > 0) {
                    i--;
                }
                continue;
            }
            // Append expanded environment variable to output
            strncat(expanded, tmp, strlen(tmp));
            free(tmp);
        }
        // Nothing to do so append input to output
        strncat(expanded, &input[i], 1);
    }

    return expanded;
}

/**
 * Set a runtime environment variable.
 *
 *
 * Note: `_value` is passed through `runtime_expand_var` to provide shell expansion
 *
 *
 * Example:
 *
 * ~~~{.c}
 * int main(int argc, char *argv[], char *arge[]) {
 *     RuntimeEnv *rt = runtime_copy(arge);
 *
 *     runtime_set(rt, "new_var", "1");
 *     char *new_var = runtime_get("new_var");
 *     // new_var = 1;
 *
 *     char *path = runtime_get("PATH");
 *     // path = /your/path:/here
 *
 *     runtime_set(rt, "PATH", "/opt/secure:$PATH");
 *     char *secure_path = runtime_get("PATH");
 *     // secure_path = /opt/secure:/your/path:/here
 *     // NOTE: path and secure_path are COPIES, unlike `getenv()` and `setenv()` that reuse their pointers in `environ`
 *
 *     runtime_free(rt);
 *     return 0;
 * }
 * ~~~
 *
 *
 * @param env `RuntimeEnv` structure
 * @param _key Environment variable to set
 * @param _value New environment variable value
 */
void runtime_set(RuntimeEnv *env, const char *_key, const char *_value) {
    if (_key == NULL) {
        return;
    }
    char *key = strdup(_key);
    ssize_t key_offset = runtime_contains(env, key);
    char *value = runtime_expand_var(env, _value);
    char *now = join((char *[]) {key, value, NULL}, "=");

    if (key_offset < 0) {
        strlist_set(env, key_offset, now);
    }
    else {
        strlist_append(env, now);
    }
    free(now);
    free(key);
    free(value);
}

/**
 * Update the global `environ` array with data from `RuntimeEnv`
 * @param env `RuntimeEnv` structure
 */
void runtime_apply(RuntimeEnv *env) {
    for (size_t i = 0; i < strlist_count(env); i++) {
        char **pair = split(strlist_item(env, i), "=");
        setenv(pair[0], pair[1], 1);
        split_free(pair);
    }
}

/**
 * Free `RuntimeEnv` allocated by `runtime_copy`
 * @param env `RuntimeEnv` structure
 */
void runtime_free(RuntimeEnv *env) {
    if (env == NULL) {
        return;
    }
    strlist_free(env);
}