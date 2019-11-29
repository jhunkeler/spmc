/**
 * SPM - Simple Package Manager
 * @file spm.c
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>

#define SYSERROR stderr, "%s:%s:%d: %s\n", __FILE__, __FUNCTION__, __LINE__, strerror(errno)
#define DIRSEP_WIN32 '\\'
#define DIRSEP_UNIX '/'
#if defined(_WIN32)
#define DIRSEP  DIRSEP_WIN32
#define NOT_DIRSEP DIRSEP_UNIX
#else
#define DIRSEP DIRSEP_UNIX
#define NOT_DIRSEP DIRSEP_WIN32
#endif

#define PKG_DIR "../pkgs"

/**
 * A wrapper for `popen()` that executes non-interactive programs and reports their exit value.
 * To redirect stdout and stderr you must do so from within the `fmt` string
 *
 * ~~~{.c}
 * int fd = 1;  // stdout
 * const char *log_file = "log.txt";
 * char *buf;
 * int status;
 *
 * // Send stderr to stdout
 * status = shell(buf, "foo 2>&1");
 * // Send stdout and stderr to /dev/null
 * status = shell(buf, "bar &>/dev/null");
 * // Send stdout from baz to log.txt
 * status = shell(buf, "baz %d>%s", fd, log_file);
 * // Do not record or redirect output from any streams
 * status = shell(NULL, "biff");
 * ~~~
 *
 * @param buf buffer to hold program output (to ignore output, set to NULL)
 * @param options change behavior of the function
 * @param fmt shell command to execute (accepts `printf` style formatters)
 * @param ... variadic arguments (used by `fmt`)
 * @return shell exit code
 */
#define SHELL_DEFAULT 1 << 0
#define SHELL_OUTPUT 1 << 1
int shell(char **buf, u_int8_t option, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    FILE *proc;
    int status = -1;
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

    proc = popen(cmd, "r");
    if (!proc) {
        return status;
    }

    size_t bytes_read = 0;
    size_t i = 0;
    size_t new_buf_size = 0;
    if (option & SHELL_OUTPUT) {
        *buf = (char *)calloc(BUFSIZ, sizeof(char));

        while (!feof(proc)) {
            *outbuf = fgetc(proc);

            if (i >= BUFSIZ) {
                new_buf_size = BUFSIZ + (i + bytes_read);
                (*buf) = (char *)realloc((*buf), new_buf_size);
                i = 0;
            }
            if (isascii(*outbuf)) {
                (*buf)[bytes_read] = *outbuf;
            }
            bytes_read++;
            i++;
        }
    }
    status = pclose(proc);
    va_end(args);
    free(outbuf);
    free(cmd);
    return status;
};

/*
int tar_extract_file(const char *archive, const char* filename, const char *destination) {
    int status = -1;
    char cmd[PATH_MAX];
    char output[3];
    sprintf(cmd, "tar xf %s %s -C %s 2>&1", archive, filename, destination);
    FILE *proc = popen(cmd, "r");
    if (!proc) {
        return status;
    }
    while (!feof(proc)) {
        fgets(output, 2, proc);
        printf("%s", output);
    }
    putchar('\n');
    return pclose(proc);
};
 */

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
    for (int i = 0; i < strlen(pattern); i++) {
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
 * This function scans `sptr` from right to left removing any matches to `suffix`
 * from the string.
 *
 * @param sptr string to be modified
 * @param suffix string to be removed from `sptr`
 */
void strip(char *sptr, const char *suffix) {
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
        strip(sptr, suffix);
    }
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

int main() {
    const char *pkg_name = "python";
    char *package = NULL;
    package = find_package(pkg_name);
    if (package != NULL) {
        printf("Package found: %s\n", package);
        free(package);
    }
    else {
        fprintf(stderr,"Package does not exist: %s\n", pkg_name);
    }

    const char *testpath = "x:\\a\\b\\c";
    const char *teststring = "this is test!";
    char *normalized = normpath(testpath);
    char *buf = NULL;

    printf("%s becomes %s\n", testpath, normalized);
    printf("%d\n", startswith(testpath, "x:\\"));
    printf("%d\n", endswith(teststring, "test!"));
    //tar_extract_file("one", "two", testpath);

    int retval = -1;
    retval = shell(&buf, SHELL_OUTPUT, "env; env; env; env; env; env; env; env");
    strip(buf, "\n");
    printf("%s\n", buf);

    free(normalized);
    return 0;
}