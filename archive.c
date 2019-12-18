#include "spm.h"

/**
 * Extract a single file from a tar archive into a directory
 *
 * @param archive path to tar archive
 * @param filename known path inside the archive to extract
 * @param destination where to extract file to (must exist)
 * @return
 */
int tar_extract_file(const char *archive, const char* filename, const char *destination) {
    Process *proc = NULL;
    int status;
    char cmd[PATH_MAX];

    sprintf(cmd, "tar xf %s -C %s %s 2>&1", archive, destination, filename);
    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        fprintf(SYSERROR);
        return -1;
    }

    status = proc->returncode;
    shell_free(proc);

    return status;
}

int tar_extract_archive(const char *_archive, const char *_destination) {
    Process *proc = NULL;
    int status;
    char cmd[PATH_MAX];

    char *archive = strdup(_archive);
    if (!archive) {
        fprintf(SYSERROR);
        return -1;
    }
    char *destination = strdup(_destination);
    if (!destination) {
        fprintf(SYSERROR);
        return -1;
    }

    // sanitize archive
    strchrdel(archive, "&;|");
    // sanitize destination
    strchrdel(destination, "&;|");

    sprintf(cmd, "tar xf %s -C %s 2>&1", archive, destination);
    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        fprintf(SYSERROR);
        free(archive);
        free(destination);
        return -1;
    }

    status = proc->returncode;
    shell_free(proc);
    free(archive);
    free(destination);
    return status;
}

