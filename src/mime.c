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
#ifdef __APPLE__
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
    output = split(proc->output, ":");
    if (!output || output[1] == NULL) {
        shell_free(proc);
        return NULL;
    }
    parts = split(output[1], ";");
    if (!parts || !parts[0] || !parts[1]) {
        shell_free(proc);
        return NULL;
    }

    char *what = strdup(parts[0]);
    what = lstrip(what);

    char *charset = strdup(strchr(parts[1], '=') + 1);
    charset = lstrip(charset);
    charset[strlen(charset) - 1] = '\0';

    char *origin = realpath(filename, NULL);

    type = (Mime *)calloc(1, sizeof(Mime));
    type->origin = origin;
    type->type = what;
    type->charset = charset;

    split_free(output);
    split_free(parts);
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
