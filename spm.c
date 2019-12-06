/**
 * SPM - Simple Package Manager
 * @file spm.c
 */

#include <ctype.h>
#include <errno.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "config.h"
#include "spm.h"



/**
 * A wrapper for `popen()` that executes non-interactive programs and reports their exit value.
 * To redirect stdout and stderr you must do so from within the `fmt` string
 *
 * ~~~{.c}
 * int fd = 1;  // stdout
 * const char *log_file = "log.txt";
 * Process *proc_info;
 * int status;
 *
 * // Send stderr to stdout
 * shell(&proc_info, SHELL_OUTPUT, "foo 2>&1");
 * // Send stdout and stderr to /dev/null
 * shell(&proc_info, SHELL_OUTPUT,"bar &>/dev/null");
 * // Send stdout from baz to log.txt
 * shell(&proc_info, SHELL_OUTPUT, "baz %d>%s", fd, log_file);
 * // Do not record or redirect output from any streams
 * shell(&proc_info, SHELL_DEFAULT, "biff");
 * ~~~
 *
 * @param Process uninitialized `Process` struct will be populated with process data
 * @param options change behavior of the function
 * @param fmt shell command to execute (accepts `printf` style formatters)
 * @param ... variadic arguments (used by `fmt`)
 */
void shell(Process **proc_info, u_int64_t option, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t bytes_read = 0;
    size_t i = 0;
    size_t new_buf_size = 0;
    clockid_t clkid = CLOCK_REALTIME;
    FILE *proc = NULL;

    (*proc_info) = (Process *)calloc(1, sizeof(Process));
    if (!(*proc_info)) {
        fprintf(SYSERROR);
        exit(errno);
    }
    (*proc_info)->returncode = -1;

    char *outbuf = (char *)calloc(1, sizeof(char));
    if (!outbuf) {
        fprintf(SYSERROR);
        exit(errno);
    }
    char *cmd = (char *)calloc(PATH_MAX, sizeof(char));
    if (!cmd) {
        fprintf(SYSERROR);
        exit(errno);
    }

    vsnprintf(cmd, PATH_MAX, fmt, args);

    if (option & SHELL_BENCHMARK) {
        if (clock_gettime(clkid, &(*proc_info)->start_time) == -1) {
            perror("clock_gettime");
            exit(errno);
        }
    }

    proc = popen(cmd, "r");
    if (!proc) {
        return;
    }

    if (option & SHELL_BENCHMARK) {
        if (clock_gettime(clkid, &(*proc_info)->stop_time) == -1) {
            perror("clock_gettime");
            exit(errno);
        }
        (*proc_info)->time_elapsed = ((*proc_info)->stop_time.tv_sec - (*proc_info)->start_time.tv_sec)
                                   + ((*proc_info)->stop_time.tv_nsec - (*proc_info)->start_time.tv_nsec) / 1E9;
    }

    if (option & SHELL_OUTPUT) {
        (*proc_info)->output = (char *)calloc(BUFSIZ, sizeof(char));

        while ((*outbuf = fgetc(proc)) != EOF) {

            if (i >= BUFSIZ) {
                new_buf_size = BUFSIZ + (i + bytes_read);
                (*proc_info)->output = (char *)realloc((*proc_info)->output, new_buf_size);
                i = 0;
            }
            if (*outbuf) {
                (*proc_info)->output[bytes_read] = *outbuf;
            }
            bytes_read++;
            i++;
        }
    }
    (*proc_info)->returncode = pclose(proc);
    va_end(args);
    free(outbuf);
    free(cmd);
}

/**
 * Free process resources allocated by `shell()`
 * @param proc_info `Process` struct
 */
void shell_free(Process *proc_info) {
    free(proc_info->output);
    free(proc_info);
}

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
    int status = -1;
    char cmd[PATH_MAX];
    char output[3];

    sprintf(cmd, "tar xf %s %s -C %s 2>&1", archive, filename, destination);

    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        printf("%s\n", proc->output);
        fprintf(SYSERROR);
        return -1;
    }
    status = proc->returncode;
    shell_free(proc);

    return status;
};

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
 * Determine how many times the character `ch` appears in `sptr` string
 * @param sptr string to scan
 * @param ch character to find
 * @return count of characters found
 */
int num_chars(const char *sptr, int ch) {
    int result = 0;
    for (int i = 0; sptr[i] != '\0'; i++) {
        if (sptr[i] == ch) {
            result++;
        }
    }
    return result;
}

/**
 * Scan for `pattern` string at the beginning of `sptr`
 *
 * @param sptr string to scan
 * @param pattern string to search for
 * @return 0 = success, -1 = failure
 */
int startswith(const char *sptr, const char *pattern) {
    for (size_t i = 0; i < strlen(pattern); i++) {
        if (sptr[i] != pattern[i]) {
            return -1;
        }
    }
    return 0;
}

/**
 * Scan for `pattern` string at the end of `sptr`
 *
 * @param sptr string to scan
 * @param pattern string to search for
 * @return 0 = success, -1 = failure
 */
int endswith(const char *sptr, const char *pattern) {
    size_t sptr_size = strlen(sptr);
    size_t pattern_size = strlen(pattern);
    for (size_t s = sptr_size - pattern_size, p = 0 ; s < sptr_size; s++, p++) {
        if (sptr[s] != pattern[p]) {
            return -1;
        }
    }
    return 0;
}

/**
 * Converts Win32 path to Unix path, and vice versa
 *  - On UNIX, Win32 paths will be converted UNIX
 *  - On Win32, UNIX paths will be converted to Win32
 *
 * This function is platform dependent.
 *
 * @param path a system path
 * @return string (caller is responsible for `free`ing memory)
 */
char *normpath(const char *path) {
    char *result = strdup(path);
    char *tmp = result;

    while (*tmp) {
        if (*tmp == NOT_DIRSEP) {
            *tmp = DIRSEP;
            tmp++;
            continue;
        }
        tmp++;
    }
    return result;
}

/**
 * Deletes any characters matching `chars` from `sptr` string
 *
 * @param sptr string to be modified in-place
 * @param chars a string containing characters (e.g. " \n" would delete whitespace and line feeds)
 */
void strchrdel(char *sptr, const char *chars) {
    while (*sptr != '\0') {
        for (int i = 0; chars[i] != '\0'; i++) {
            if (*sptr == chars[i]) {
                memmove(sptr, sptr + 1, strlen(sptr));
            }
        }
        sptr++;
    }
}

int64_t strchroff(char *sptr, int ch) {
    char *tmp = sptr;
    while (*tmp != '\0') {
        if (*tmp == ch) {
            return tmp - sptr;
        }
        tmp++;
    }
    return -1;
}
/**
 * This function scans `sptr` from right to left removing any matches to `suffix`
 * from the string.
 *
 * @param sptr string to be modified
 * @param suffix string to be removed from `sptr`
 */
void substrdel(char *sptr, const char *suffix) {
    if (!sptr || !suffix) {
        return;
    }
    size_t sptr_len = strlen(sptr);
    size_t suffix_len = strlen(suffix);
    intptr_t target_offset = sptr_len - suffix_len;

    // Prevent access to memory below input string
    if (target_offset < 0) {
        return;
    }

    // Create a pointer to
    char *target = sptr + target_offset;
    if (!strcmp(target, suffix)) {
        // Purge the suffix
        memset(target, '\0', suffix_len);
        // Recursive call continues removing suffix until it is gone
        strip(sptr);
    }
}

char *find_file(const char *root, const char *filename) {
    glob_t results;
    int glob_flags = 0;
    int match = 0;
    char *rootpath = NULL;
    char *path = NULL;

    // GUARD
    if (!root || !filename || strstr(filename, "..") || strstr(filename, "./")) {
        return NULL;
    }

    if (!(path = (char *)calloc(PATH_MAX + 1, sizeof(char)))) {
        fprintf(SYSERROR);
        exit(errno);
    }

    if (!(rootpath = realpath(root, NULL))) {
        return NULL;
    }

    strcat(path, rootpath);
    strcat(path, "/");
    strcat(path, filename);

    // Save a little time if the file exists
    if (access(path, F_OK) != -1) {
        return path;
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
        return NULL;
    }

    // Resize path to the length of the first match
    char *want = results.gl_pathv[0];
    if (!(path = (char *)realloc(path, strlen(want) + 1))) {
        fprintf(SYSERROR);
        exit(errno);
    }

    // Replace path string with wanted path string
    strncpy(path, want, strlen(want));

    free(rootpath);
    globfree(&results);
    return path;
}

char *find_package(const char *filename) {
    return find_file(PKG_DIR, filename);
}

char** split(char *sptr, const char* delim)
{
    int split_alloc = 0;
    for (int i = 0; i < strlen(delim); i++) {
        split_alloc += num_chars(sptr, delim[i]);
    }
    char **result = (char **)calloc(split_alloc + 2, sizeof(char *));

    int i = 0;
    //char *token = strsep(&sptr, delim);
    char *token = NULL;
    while((token = strsep(&sptr, delim)) != NULL) {
        result[i] = (char *)calloc(1, sizeof(char) * strlen(token) + 1);
        strcpy(result[i], token);
        i++;
    }
    return result;
}

void split_free(char **ptr) {
    for (int i = 0; ptr[i] != NULL; i++) {
        free(ptr[i]);
    }
    free(ptr);
}

char *substring_between(char *sptr, const char *delims) {
    int delim_count = strlen(delims);
    if (delim_count != 2) {
        return NULL;
    }

    char *start = strpbrk(sptr, &delims[0]);
    char *end = strpbrk(sptr, &delims[1]);
    if (!start || !end) {
        return NULL;
    }
    start++;    // ignore leading delimiter

    long int length = end - start;

    char *result = (char *)calloc(1, sizeof(char) * length);
    if (!result) {
        return NULL;
    }

    char *tmp = result;
    while (start != end) {
        *tmp = *start;
        tmp++;
        start++;
    }

    return result;
}

/**
 * Uses `readelf` to determine whether a RPATH or RUNPATH is present
 *
 * TODO: Replace with OS-native solution(s)
 *
 * @param filename path to executable or library
 * @return -1=OS error, 0=has rpath, 1=not found
 */
int has_rpath(const char *filename) {
    int result = 1;
    Process *proc_info = NULL;
    char *path = strdup(filename);
    const char *preamble = "readelf -d";

    // sanitize input path
    strchrdel(path, "&;|");

    char sh_cmd[strlen(preamble) + 1 + strlen(path) + 1];
    memset(sh_cmd, '\0', sizeof(sh_cmd));

    sprintf(sh_cmd, "%s %s", preamble, path);
    free(path);

    shell(&proc_info, SHELL_OUTPUT, sh_cmd);
    if (!proc_info) {
        fprintf(SYSERROR);
        return -1;
    }

   strip(proc_info->output);
   char **lines = split(proc_info->output, "\n");
   for (int i = 0; lines[i] != NULL; i++) {
       if (strstr(lines[i], "RPATH") || strstr(lines[i], "RUNPATH")) {
           result = 0;
       }
   }
    shell_free(proc_info);
   split_free(lines);
   return result;
}

/**
 * Parses `readelf` output and returns an RPATH or RUNPATH if one is defined
 *
 * TODO: Replace with OS-native solution(s)
 *
 * @param filename path to executable or library
 * @return RPATH string, NULL=error (caller is responsible for freeing memory)
 */
char *get_rpath(const char *filename) {
    if ((has_rpath(filename)) != 0) {
        return NULL;
    }

    Process *proc_info = NULL;
    char *path = strdup(filename);
    const char *preamble = "readelf -d";
    char *rpath = NULL;

    // sanitize input path
    strchrdel(path, "&;|");

    char sh_cmd[strlen(preamble) + 1 + strlen(path) + 1];
    memset(sh_cmd, '\0', sizeof(sh_cmd));

    sprintf(sh_cmd, "%s %s", preamble, path);
    free(path);

    shell(&proc_info, SHELL_OUTPUT, sh_cmd);
    if (!proc_info) {
        fprintf(SYSERROR);
        return NULL;
    }

    strip(proc_info->output);
    char **lines = split(proc_info->output, "\n");
    for (int i = 0; lines[i] != NULL; i++) {
        if (strstr(lines[i], "RPATH") || strstr(lines[i], "RUNPATH")) {
            rpath = substring_between(lines[i], "[]");
            break;
        }
    }
    shell_free(proc_info);
    split_free(lines);
    return rpath;
}

int main(int argc, char *argv[]) {
    // not much to see here yet
    // at the moment this will all be random tests, for better or worse
    // everything here is subject to change without notice

    const char *pkg_name = "python";
    char *package = NULL;
    package = find_package(pkg_name);
    if (package != NULL) {
        printf("Package found: %s\n", package);
        free(package);
    } else {
        fprintf(stderr, "Package does not exist: %s\n", pkg_name);
    }

    const char *testpath = "x:\\a\\b\\c";
    const char *teststring = "this is test!";
    const char *testprog = "/tmp/a.out";
    char *normalized = normpath(testpath);

    printf("%s becomes %s\n", testpath, normalized);
    printf("%d\n", startswith(testpath, "x:\\"));
    printf("%d\n", endswith(teststring, "test!"));
    free(normalized);

    if ((has_rpath(testprog)) == 0) {
        printf("RPATH found\n");
        char *rpath = get_rpath(testprog);
        printf("RPATH is: %s\n", rpath);
        free(rpath);
    }
    return 0;
}