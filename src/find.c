/**
 * @file find.c
 */
#include "spm.h"

/**
 * glob callback function
 * @param epath path to file that generated the error condition
 * @param eerrno the error condition
 * @return the error condition
 */
int errglob(const char *epath, int eerrno) {
    fprintf(stderr, "glob matching error: %s (%d)", epath, eerrno);
    return eerrno;
}

/**
 * Scan a directory for a file by name, or by wildcard
 *
 * @param root directory path to scan
 * @param filename file to find (wildcards accepted)
 * @return success=path to file, failure=NULL
 */
char *find_file(const char *root, const char *filename) {
    glob_t results;
    int glob_flags = 0;
    int match = 0;
    char path[PATH_MAX];
    memset(path, '\0', PATH_MAX);

    // GUARD
    if (!root || !filename || strstr(filename, "..") || strstr(filename, "./")) {
        return NULL;
    }

    if (realpath(root, path) == NULL) {
        perror("Cannot determine realpath()");
        fprintf(SYSERROR);
        return NULL;
    }

    strcat(path, "/");
    strcat(path, filename);

    // Save a little time if the file exists
    if (access(path, F_OK) != -1) {
        return strdup(path);
    }

    // Inject wildcard
    strcat(path, "*");
    // Search for the file
    match = glob(path, glob_flags, errglob, &results);

    if (match != 0) {
        // report critical errors except GLOB_NOMATCH
        if (match == GLOB_NOSPACE || match == GLOB_ABORTED) {
            fprintf(SYSERROR);
        }
        globfree(&results);
        return NULL;
    }

    // Replace path string with wanted path string
    strcpy(path, results.gl_pathv[0]);

    globfree(&results);
    return strdup(path);
}

/**
 * Scan the package directory for a package by name
 * @param filename file to find
 * @return success=path to file, failure=NULL
 */
char *find_package(const char *filename) {
    char *repo = join((char *[]) {SPM_GLOBAL.package_dir, SPM_GLOBAL.repo_target, NULL}, DIRSEPS);
    char *match = find_file(repo, filename);
    free(repo);
    return match;
}

/**
 * Determine whether `pattern` is present within a file
 * @param filename
 * @param pattern
 * @return 0=found, 1=not found, -1=OS error
 */
int find_in_file(const char *filename, const char *pattern) {
    int result = 1;  // default "not found"

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return -1;
    }

    long int file_len = get_file_size(filename);
    if (file_len == -1) {
        fclose(fp);
        return -1;
    }
    char *buffer = (char *)calloc((size_t) file_len, sizeof(char));
    if (!buffer) {
        fclose(fp);
        return -1;
    }
    size_t pattern_len = strlen(pattern);

    fread(buffer, (size_t) file_len, sizeof(char), fp);
    fclose(fp);

    for (size_t i = 0; i < (size_t) file_len; i++) {
        if (!memcmp(&buffer[i], pattern, pattern_len)) {
            result = 0;  // found
            break;
        }
    }
    free(buffer);
    return result;
}

/**
 * Get the full path of a shell command
 * @param program
 * @return success=absolute path to program, failure=NULL
 */
char *find_executable(const char *program) {
    int found = 0;
    char *result = NULL;
    char *env_path = NULL;
    env_path = getenv("PATH");
    if (!env_path) {
        return NULL;
    }
    char **search_paths = split(env_path, ":");

    char buf[PATH_MAX];
    for (int i = 0; search_paths[i] != NULL; i++) {
        sprintf(buf, "%s%c%s", search_paths[i], DIRSEP, program);
        if (access(buf, F_OK | X_OK) == 0) {
            found = 1;
            break;
        }
        memset(buf, '\0', sizeof(buf));
    }
    if (found) {
        result = strdup(buf);
    }
    split_free(search_paths);
    return result;
}
