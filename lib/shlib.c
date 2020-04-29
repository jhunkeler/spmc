#include "spm.h"
#include "shlib.h"

char *objdump(const char *_filename, char *_args) {
    // do not expose this function
    char *filename = NULL;
    char *result = NULL;
    Process *proc = NULL;
    char cmd[PATH_MAX];
    memset(cmd, '\0', sizeof(cmd));

    if (_filename == NULL) {
        spmerrno = EINVAL;
        spmerrno_cause("_filename was NULL");
        return NULL;
    }

    if ((filename = strdup(_filename)) == NULL) {
        fprintf(SYSERROR);
        return NULL;
    }

    strchrdel(filename, SHELL_INVALID);
    snprintf(cmd, sizeof(cmd), "%s %s '%s'", SPM_SHLIB_EXEC, _args, filename);
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

char *shlib_rpath(const char *filename) {
    char **data = NULL;
    char *raw_data = NULL;
    char *result = NULL;

    if (filename == NULL) {
        spmerrno = EINVAL;
        spmerrno_cause("filename was NULL");
        return NULL;
    }

    if ((raw_data = objdump(filename, SPM_SHLIB_EXEC_ARGS)) == NULL) {
        return NULL;
    }

    // Split output into individual lines
    if ((data = split(raw_data, "\n")) == NULL) {
        free(raw_data);
        return NULL;
    }

    // Collapse all whitespace in each line
    // i.e. "    stuff  things" -> "stuff  things"
    for (size_t i = 0; data[i] != NULL; i++) {
        data[i] = normalize_space(data[i]);
    }

    for (size_t i = 0; data[i] != NULL; i++) {
        char **field = NULL;
        char reason[255] = {0,};

#if OS_LINUX
        // Extract the RPATH record (second field)
        if (startswith(data[i], "RPATH")) {
            if ((field = split(data[i], " ")) == NULL) {
                break;
            }

            // record library path
            result = strdup(field[1]));
            split_free(field);
            break;
        }
#elif OS_DARWIN
        size_t offset_name = i + 2;  // how many lines to look ahead after reaching LC_RPATH
        size_t numLines;
        for (numLines = 0; data[numLines] != NULL; numLines++); // get line count

        // Find APPLE's equivalent to RPATH on Linux
        if (startswith(data[i], "cmd LC_RPATH")) {
            // Don't overrun the data buffer
            if (offset_name > numLines || data[offset_name] == NULL) {
                break;
            }

            // split on: "path /library/path"
            if ((field = split(data[offset_name], " ")) == NULL) {
                sprintf(reason, "'%s' produced unreadable output at offset %zu", SPM_SHLIB_EXEC, offset_name);
                spmerrno = SPM_ERR_PARSE;
                spmerrno_cause(reason);
                break;
            }

            // verify it was actually "path ..."
            if (strcmp(field[0], "path") != 0) {
                sprintf(reason, "'%s' produced unexpected LC_RPATH format between lines %zu:%zu", SPM_SHLIB_EXEC, i, offset_name);
                spmerrno = SPM_ERR_PARSE;
                spmerrno_cause(reason);
                break;
            }

            // record library path
            result = strdup(field[1]);
            split_free(field);
            break;
        }
#endif
    }

    free(raw_data);
    split_free(data);
    return result;
}

StrList *shlib_deps(const char *filename) {
    char **data = NULL;
    char *raw_data = NULL;
    StrList *result = NULL;

    if (filename == NULL) {
        spmerrno = EINVAL;
        spmerrno_cause("filename was NULL");
        return NULL;
    }

    // Get output from objdump
    if ((raw_data = objdump(filename, SPM_SHLIB_EXEC_ARGS)) == NULL) {
        return NULL;
    }

    // Initialize list array
    if ((result = strlist_init()) == NULL) {
        free(raw_data);
        return NULL;
    }

    // Split output into individual lines
    if ((data = split(raw_data, "\n")) == NULL) {
        free(raw_data);
        strlist_free(result);
        return NULL;
    }

    // Collapse all whitespace in each line
    // i.e. "    stuff  things" -> "stuff  things"
    for (size_t i = 0; data[i] != NULL; i++) {
        data[i] = normalize_space(data[i]);
    }

    for (size_t i = 0; data[i] != NULL; i++) {
        char **field = NULL;
        char reason[255] = {0,};

#if OS_LINUX
        // Extract the NEEDED libraries (second field)
        // AFAIK when "NEEDED" is present, a string containing the library name is guaranteed to be there
        if (startswith(data[i], "NEEDED")) {
            if ((field = split(data[i], " ")) == NULL) {
                strlist_free(result);
                result = NULL;
                break;
            }

            // record library path
            strlist_append(result, field[1]);
            split_free(field);
        }
#elif OS_DARWIN
        size_t offset_name = i + 2;  // how many lines to look ahead after reaching LC_LOAD_DYLIB
        size_t numLines;
        for (numLines = 0; data[numLines] != NULL; numLines++); // get line count

        // Find APPLE's equivalent to NEEDED on Linux
        if (startswith(data[i], "cmd LC_LOAD_DYLIB")) {
            // Don't overrun the data buffer
            if (offset_name > numLines || data[offset_name] == NULL) {
                break;
            }

            // split on: "name /library/path"
            if ((field = split(data[offset_name], " ")) == NULL) {
                sprintf(reason, "'%s' produced unreadable output at offset %zu", SPM_SHLIB_EXEC, offset_name);
                spmerrno = SPM_ERR_PARSE;
                spmerrno_cause(reason);

                strlist_free(result);
                result = NULL;
                break;
            }

            // verify it was actually "name ..."
            if (strcmp(field[0], "name") != 0) {
                sprintf(reason, "'%s' produced unexpected LC_LOAD_DYLIB format between lines %zu:%zu", SPM_SHLIB_EXEC, i, offset_name);
                spmerrno = SPM_ERR_PARSE;
                spmerrno_cause(reason);
                break;
            }

            // record library path
            strlist_append(result, field[1]);
            split_free(field);
        }
#endif
    }

    free(raw_data);
    split_free(data);
    return result;
}
