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
    const char *root = "/tmp/root";
    const char *package = "python";

    Dependencies *deps = NULL;
    dep_init(&deps);
    dep_all(&deps, package);
    printf("%s requires:\n", package);
    dep_show(&deps);

    // Install dependencies first
    for (int i = 0; i < deps->records; i++) {
        install(root, deps->list[i]);
    }
    // Install package
    install(root, package);

    dep_free(&deps);
    free_global_config();
    return 0;
}
