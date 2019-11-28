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

int shell(char *buf, const char *fmt, ...) {
    char *buf_orig = buf;
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

    vsnprintf(cmd, sizeof(outbuf), fmt, args);

    proc = popen(cmd, "r");
    if (!proc) {
        return status;
    }

    if (buf != NULL) {
        while (!feof(proc)) {
            *outbuf = fgetc(proc);
            if (isascii(*outbuf)) {
                *buf = *outbuf;
            }
            buf++;
        }
        buf = buf_orig;
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

int errglob(const char *epath, int eerrno) {
    fprintf(stderr, "glob matching error: %s (%d)", epath, eerrno);
    return eerrno;
}

int num_chars(const char *sptr, int ch) {
    int result = 0;
    for (int i = 0; sptr[i] != '\0'; i++) {
        if (sptr[i] == ch) {
            result++;
        }
    }
    return result;
}

int startswith(const char *sptr, const char *pattern) {
    for (int i = 0; i < strlen(pattern); i++) {
        if (sptr[i] != pattern[i]) {
            return -1;
        }
    }
    return 0;
}

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

char *normpath(const char *path) {
    // Convert Win32 path to Unix path, and vice-versa
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
 * This function scans `sptr` from right to left removing any matches to `suffix` from `sptr`
 *
 * @param sptr string to be modified
 * @param suffix string to be removed from `sptr`
 */
void strip(char *sptr, const char *suffix) {
    size_t sptr_len = strlen(sptr);                     //!< length of sptr
    size_t suffix_len = strlen(suffix);                 //!< length of suffix
    intptr_t target_offset = sptr_len - suffix_len;     //!< offset to possible suffix

    // Prevent access to memory below input string
    if (target_offset < 0) {
        return;
    }

    // Create a pointer to where the suffix should be
    char *target = sptr + target_offset;                //!< pointer to possible suffix
    if (!strcmp(target, suffix)) {
        // Purge the suffix
        memset(target, '\0', suffix_len);
        // Recursive call continues removing suffix until they are gone
        strip(sptr, suffix);
    }
}

/**
 * Deletes any characters matching `chars` from `sptr` string
 *
 * @param s string to be modified in-place
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
    char buf[10240];
    memset(buf, 0, sizeof(buf));

    printf("%s becomes %s\n", testpath, normalized);
    printf("%d\n", startswith(testpath, "x:\\"));
    printf("%d\n", endswith(teststring, "test!"));
    //tar_extract_file("one", "two", testpath);

    int retval = -1;
    retval = shell(buf, "env");
    strip(buf, "\n");
    printf("%s\n", buf);

    free(normalized);
    return 0;
}