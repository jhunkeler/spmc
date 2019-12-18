/**
 * SPM - Simple Package Manager
 * @file spm.c
 */
#include "spm.h"

int main(int argc, char *argv[]) {
    // not much to see here yet
    // at the moment this will all be random tests, for better or worse
    // everything here is subject to change without notice

    // Initialize configuration data
    init_config_global();
    show_global_config();

    // Ensure external programs are available for use.
    check_runtime_environment();

    // Install a package to test things out
    char *match;
    char *package;
    const char *root = "/tmp/root";
    if ((match = find_package("python")) == NULL) {
        fprintf(SYSERROR);
        exit(1);
    }
    if ((package = basename(match)) == NULL) {
        fprintf(stderr, "Unable to derive package name from package path:\n\t-> %s\n", match);
        exit(1);
    }

    Dependencies *deps = NULL;
    dep_init(&deps);

    if (dep_all(&deps, package) < 0) {
        dep_free(&deps);
        free_global_config();
        exit(1);
    }

    printf("%s requires:\n", package);
    dep_show(&deps);

    // Install dependencies first
    for (int i = 0; i < deps->records; i++) {
        if (install(root, deps->list[i]) < 0) {
            fprintf(SYSERROR);
            exit(errno);
        }
    }
    // Install package
    if (install(root, package) < 0) {
        fprintf(SYSERROR);
        exit(errno);
    }

    dep_free(&deps);
    free_global_config();
    return 0;
}
