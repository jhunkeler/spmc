#include "spm.h"
#include "shlib.h"

char *shlib_deps_objdump(const char *_filename) {
    // do not expose this function
    char *filename = NULL;
    char *result = NULL;
    Process *proc = NULL;
    char cmd[PATH_MAX];
    memset(cmd, '\0', sizeof(cmd));

    if ((filename = strdup(_filename)) == NULL) {
        fprintf(SYSERROR);
        return NULL;
    }

    strchrdel(filename, SHELL_INVALID);
    snprintf(cmd, sizeof(cmd), "%s %s '%s'", "objdump", "-p", filename);
    shell(&proc, SHELL_OUTPUT, cmd);

    if (proc->returncode != 0) {
        free(filename);
        shell_free(proc);
        return NULL;
    }
    result = strdup(proc->output);

    free(filename);
    shell_free(proc);
    return result;
}

StrList *shlib_deps(const char *filename) {
    char **data = NULL;
    char *output = NULL;
    StrList *result = NULL;

    // Get output from objdump
    // TODO: use preprocessor or another function to select the correct shlib_deps_*() in the future
    if ((output = shlib_deps_objdump(filename)) == NULL) {
        return NULL;
    }

    // Initialize list array
    if ((result = strlist_init()) == NULL) {
        free(output);
        return NULL;
    }

    // Split output into individual lines
    if ((data = split(output, "\n")) == NULL) {
        free(output);
        strlist_free(result);
        return NULL;
    }

    // Parse output:
    // Collapse whitespace and extract the NEEDED libraries (second field)
    // AFAIK when "NEEDED" is present, a string containing the library name is guaranteed to be there
    for (size_t i = 0; data[i] != NULL; i++) {
        data[i] = normalize_space(data[i]);
        if (startswith(data[i], "NEEDED") == 0) {
            char **field = split(data[i], " ");
            strlist_append(result, field[1]);
            split_free(field);
        }
    }

    free(output);
    split_free(data);
    return result;
}
