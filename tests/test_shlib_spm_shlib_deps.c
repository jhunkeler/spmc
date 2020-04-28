#include "spm.h"
#include "framework.h"

static char *LIBRARY_SEARCH_PATH[] = {
#if OS_DARWIN
        "/usr/lib",
        "/usr/local/lib",
#elif OS_LINUX
        "/lib",
        "/usr/lib",
        "/usr/local/lib",
        "/lib64",
        "/usr/lib64",
        "/usr/local/lib64",
#endif
        NULL,
};

/**
 * Find a library based on known library search paths. This cannot handle macos-style `@string` paths, and
 * will not follow RPATHs.
 * @param name
 * @return path to library (or NULL)
 */
static char *find_library(const char *name) {
    char *path = NULL;

    if (strstr(name, DIRSEPS) != NULL) {
        return strdup(name);
    }

    for (size_t i = 0; LIBRARY_SEARCH_PATH[i] != NULL; i++) {
        path = join_ex(DIRSEPS, LIBRARY_SEARCH_PATH[i], name, NULL);
        if (path != NULL && exists(path) == 0) {
            break;
        }
        free(path);
        path = NULL;
    }
    return path;
}

struct TestCase testCase[] = {
        {.caseValue.sptr = "/bin/sh", .truthValue.signed_int = 0},
        {.caseValue.sptr = "/usr/bin/tar", .truthValue.signed_int = 0},
        {.caseValue.sptr = "/dev/null", .truthValue.signed_int = -1}, // not an object
        {.caseValue.sptr = NULL, .truthValue.signed_int = -1}, // invalid call
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);


int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        StrList *result = shlib_deps(testCase[i].caseValue.sptr);
        if (result == NULL && testCase[i].truthValue.signed_int < 0) {
            // expected failure
            fprintf(stderr, "case %zu: trapped expected failure (ignore any stderr text)\n", i);
            continue;
        }

        myassert(spmerrno == 0, "case %zu: raised unhandled exception %d: %s\n", i, spmerrno, spm_strerror(spmerrno));
        myassert(result != NULL, "case %zu: unexpected NULL", i);
        for (size_t j = 0; j < strlist_count(result);  j++) {
            char *item = strlist_item(result, j);
            char *libpath = find_library(item);
            myassert(libpath != NULL,
                    "library record found, but does not exist: '%s' (your OS is broken)\n", item);
            free(libpath);
        }
        strlist_free(result);
    }
    return 0;
}