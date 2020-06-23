/**
 * @file archive.c
 */
#include "spm.h"

/**
 * Extract a single file from a tar archive into a directory
 *
 * @param archive path to tar archive
 * @param filename known path inside the archive to extract
 * @param destination where to extract file to (must exist)
 * @return
 */
int tar_extract_file(const char *_archive, const char* _filename, const char *_destination) {
    Process *proc = NULL;
    int status;
    char cmd[PATH_MAX];
    char *archive = strdup(_archive);
    if (!archive) {
        fprintf(SYSERROR);
        return -1;
    }
    char *filename = strdup(_filename);
    if (!filename) {
        fprintf(SYSERROR);
        return -1;
    }
    char *destination = strdup(_destination);
    if (!destination) {
        fprintf(SYSERROR);
        return -1;
    }

    strchrdel(archive, SHELL_INVALID);
    strchrdel(destination, SHELL_INVALID);
    strchrdel(filename, SHELL_INVALID);

    sprintf(cmd, "%s -x -f \"%s\" -C \"%s\" \"%s\" 2>&1", SPM_GLOBAL.tar_program, archive, destination, filename);
    if (exists(archive) != 0) {
        fprintf(stderr, "unable to find archive: %s\n", archive);
        fprintf(SYSERROR);
        return -1;
    }

    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        fprintf(SYSERROR);
        return -1;
    }

    status = proc->returncode;
    if (status != 0) {
        fprintf(stderr, "%s\n", proc->output);
    }

    shell_free(proc);
    free(archive);
    free(filename);
    free(destination);
    return status;
}

/**
 *
 * @param _archive
 * @param _destination
 * @return
 */
int tar_extract_archive(const char *_archive, const char *_destination) {
    Process *proc = NULL;
    int status;
    char cmd[PATH_MAX];

    if (exists(_archive) != 0) {
        //fprintf(SYSERROR);
        return -1;
    }

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
    strchrdel(archive, SHELL_INVALID);
    // sanitize destination
    strchrdel(destination, SHELL_INVALID);

    sprintf(cmd, "%s -x -f %s -C %s ", SPM_GLOBAL.tar_program, archive, destination);
    if (SPM_GLOBAL.privileged) {
        // Extract the package as if we were a regular user instead of root.
        strcat(cmd, "--no-same-owner --no-same-permissions ");
    }
    strcat(cmd, "2>&1");

    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        fprintf(SYSERROR);
        free(archive);
        free(destination);
        return -1;
    }

    status = proc->returncode;
    if (status != 0 && SPM_GLOBAL.verbose) {
        fprintf(stderr, "%s", proc->output);
    }

    shell_free(proc);
    free(archive);
    free(destination);
    return status;
}

