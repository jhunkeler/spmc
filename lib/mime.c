/**
 * @file mime.c
 */
#include "spm.h"
#include <fnmatch.h>

/**
 * Execute OS `file` command
 * @param _filename path to file
 * @return Process structure
 */
Process *file_command(const char *_filename) {
    char *filename = strdup(_filename);
    Process *proc_info = NULL;
    char sh_cmd[PATH_MAX];
    sh_cmd[0] = '\0';
#if OS_DARWIN
    const char *fmt_cmd = "file -I \"%s\" 2>&1";
#else  // GNU
    const char *fmt_cmd = "file -i \"%s\" 2>&1";
#endif
    const char *fail_pattern = ": cannot open";

    strchrdel(filename, SHELL_INVALID);
    sprintf(sh_cmd, fmt_cmd, filename);
    shell(&proc_info, SHELL_OUTPUT, sh_cmd);

    // POSIXly ridiculous. Return non-zero when a file can't be found, or isn't accessible
    if (strstr(proc_info->output, fail_pattern) != NULL) {
        proc_info->returncode = 1;
    }
    free(filename);
    return proc_info;
}

/**
 * Execute the `file` command, parse its output, and return the data in a `Mime` structure
 * @param filename path to file
 * @return Mime structure
 */
Mime *file_mimetype(const char *filename) {
    char **output = NULL;
    char **parts = NULL;
    Mime *type = NULL;
    Process *proc = file_command(filename);

    if (proc->returncode != 0) {
        fprintf(stderr, "%s\n", proc->output);
        fprintf(stderr, "file command returned: %d\n", proc->returncode);
        fprintf(SYSERROR);
        shell_free(proc);
        return NULL;
    }

#if OS_DARWIN
    if (proc->output) {
        // Universal binaries spit out multiple lines, but we only require the first
        char *terminate_multi_part = strchr(proc->output, '\n');
        if (terminate_multi_part != NULL) {
            *(terminate_multi_part + 1) = '\0';
        }
    }
#endif

    output = split(proc->output, ":");
    if (!output || output[1] == NULL) {
        shell_free(proc);
        return NULL;
    }

    char *origin = NULL;
    char *what = NULL;
    char *charset = NULL;

    if (strchr(output[1], ';')) {
        parts = split(output[1], ";");
        if (!parts || !parts[0] || !parts[1]) {
            shell_free(proc);
            return NULL;
        }

        what = strdup(parts[0]);
        what = lstrip(what);

        charset = strdup(strchr(parts[1], '=') + 1);
        charset = lstrip(charset);
        charset[strlen(charset) - 1] = '\0';
    } else {
        // this branch is for Darwin; the 'charset=' string is not guaranteed to exist using argument `-I`
        what = strdup(output[1]);
        what = lstrip(what);
        what = strip(what);
        if (strstr(output[1], "binary") != NULL) {
            charset = strdup("binary");
        }
    }

    origin = realpath(filename, NULL);

    type = (Mime *)calloc(1, sizeof(Mime));
    type->origin = origin;
    type->type = what;
    type->charset = charset;

    split_free(output);
    if (parts != NULL) {
        split_free(parts);
    }
    shell_free(proc);
    return type;
}

/**
 * Free a `Mime` structure
 * @param m
 */
void mime_free(Mime *m) {
    if (m != NULL) {
        free(m->origin);
        free(m->type);
        free(m->charset);
        free(m);
    }
}

/**
 * Determine if a file is a text file
 * @param filename
 * @return yes=1, no=0
 */
int file_is_text(const char *filename) {
    int result = 0;
    char *path = normpath(filename);
    Mime *type = file_mimetype(path);
    if (type == NULL) {
        fprintf(stderr, "type detection failed: %s\n", filename);
        return -1;
    }
    if (startswith(type->type, "text/")) {
        result = 1;
    }
    free(path);
    mime_free(type);
    return result;
}

/**
 * Determine if a file is a binary data file
 * @param filename
 * @return yes=1, no=0
 */
int file_is_binary(const char *filename) {
    int result = 0;
    char *path = normpath(filename);
    Mime *type = file_mimetype(path);
    if (type == NULL) {
        fprintf(stderr, "type detection failed: %s\n", filename);
        return -1;
    }
    if (startswith(type->type, "application/") && strcmp(type->charset, "binary") == 0) {
        result = 1;
    }
    free(path);
    mime_free(type);
    return result;
}

int file_is_binexec(const char *filename) {
    int result = 0;
    char *path = normpath(filename);
    Mime *type = file_mimetype(path);
    if (type == NULL) {
        fprintf(stderr, "type detection failed: %s\n", filename);
        return -1;
    }
    // file-5.38: changed mime name associated with executables
    // TODO: implement compatibility function to return the correct search pattern
    if (fnmatch("application/x-[pic|pie|ex|sh]*", type->type, FNM_PATHNAME) != FNM_NOMATCH && strcmp(type->charset, "binary") == 0) {
        result = 1;
    }
    free(path);
    mime_free(type);
    return result;
}
