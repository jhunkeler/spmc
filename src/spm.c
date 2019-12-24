/**
 * SPM - Simple Package Manager
 * @file spm.c
 */
#include <errno.h>
#include "spm.h"

int RUNTIME_INSTALL = 0;
int RUNTIME_ROOTDIR = 0;
const int PACKAGE_MAX = 0xff;

void usage(const char *program_name) {
    printf(
            "usage: %s [-hVv] [-I|--install {package ...}]\n"
            "  -h,  --help     show this help message\n"
            "  -V,  --version  show version\n"
            "  -v,  --verbose  show more information\n"
            "  -I,  --install  install package(s)\n"
            "  -r   --root     installation prefix (requires --install)\n"
            , program_name
    );
}

int main(int argc, char *argv[]) {
    char program_name[strlen(argv[0]) + 1];
    memset(program_name, '\0', sizeof(program_name) + 1);
    strcpy(program_name, basename(argv[0]));

    // not much to see here yet
    // at the moment this will all be random tests, for better or worse
    // everything here is subject to change without notice

    // Initialize configuration data
    init_config_global();

    // Ensure external programs are available for use.
    check_runtime_environment();

    char root[PATH_MAX];
    memset(root, '\0', PATH_MAX);

    char *packages[PACKAGE_MAX];
    memset(packages, '\0', sizeof(char *));

    if (argc < 2) {
        usage(program_name);
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        char *arg_next = argv[i + 1] ? argv[i + 1] : NULL;

        // options
        if (*arg == '-' || strncmp(arg, "--", 2) == 0) {
            if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
                usage(program_name);
                exit(0);
            }
            else if (strcmp(arg, "-V") == 0 || strcmp(arg, "--version") == 0) {
                printf("want version\n");
                exit(0);
            }
            else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
                SPM_GLOBAL.verbose = 1;
            }
            else if (strcmp(arg, "--reindex") == 0) {
                Manifest *info = manifest_from(SPM_GLOBAL.package_dir);
                manifest_write(info);
                manifest_free(info);
                exit(0);
            }
            else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--root") == 0) {
                RUNTIME_ROOTDIR = 1;
                if (!arg_next) {
                    fprintf(stderr, "-r|--root requires a path\n");
                    usage(program_name);
                    exit(1);
                }
                strcpy(root, arg_next);
                i++;
            }
            else if (strcmp(arg, "-I") == 0 || strcmp(arg, "--install") == 0) {
                RUNTIME_INSTALL = 1;
                for (int p = 0; i < argc; p++) {
                    i++;
                    if (startswith(argv[i], "-") == 0 || startswith(argv[i], "--") == 0) {
                        if (!p) {
                            fprintf(stderr, "-I|--install requires at least one package (got: '%s')\n", argv[i]);
                            exit(1);
                        }
                        i--;
                        break;
                    }
                    packages[p] = argv[i];
                }
            }
        }
        else {
            printf("Unknown option: %s\n", arg);
            usage(program_name);
            exit(1);
        }
    }

    // Dump configuration
    if (SPM_GLOBAL.verbose) {
        show_global_config();
    }

    if (RUNTIME_ROOTDIR && !RUNTIME_INSTALL) {
        fprintf(stderr, "-r|--root requires -I|--install\n");
        usage(program_name);
        exit(1);
    }

    if (RUNTIME_INSTALL) {
        Dependencies *deps = NULL;
        dep_init(&deps);

        printf("Reading package manifest... ");
        Manifest *manifest = manifest_read();
        if (!manifest) {
            fprintf(stderr, "Package manifest is missing or corrupt\n");
            exit(1);
        }
        printf("done\n");

        if (isempty(root)) {
            printf("Using default installation root\n");
            sprintf(root, "%s%c%s", getenv("HOME"), DIRSEP, "spm_root");
        }

        printf("Installation root: %s\n", root);

        printf("Requested packages:\n");
        for (int i = 0; i < PACKAGE_MAX; i++) {
            if (packages[i] == NULL) {
                break;
            }
            printf("  -> %s\n", packages[i]);
        }

        printf("Resolving package requirements...\n");
        for (int i = 0; i < PACKAGE_MAX; i++) {
            ManifestPackage *package = NULL;
            if (packages[i] == NULL) {
                break;
            }

            package = manifest_search(manifest, packages[i]);

            // If the package has dependencies listed, append them to `deps` now
            if (package && package->requirements) {
                for (int p = 0; package->requirements[p] != NULL; p++) {
                    dep_append(&deps, package->requirements[p]);
                }
            }
            /*
            if ((match = find_package(packages[i])) == NULL) {
                fprintf(SYSERROR);
                exit(1);
            }

            if ((package = basename(match)) == NULL) {
                fprintf(stderr, "Unable to derive package name from package path:\n\t-> %s\n", match);
                exit(1);
            }
            */

            if (dep_all(&deps, package->archive) < 0) {
                dep_free(&deps);
                free_global_config();
                exit(1);
            }
        }

        if (deps) {
            // List requirements before installation
            for (int i = 0; i < deps->records; i++) {
                printf("  -> %s\n", deps->list[i]);
            }

            printf("Installing package requirements:\n");
            for (int i = 0; i < deps->records; i++) {
                printf("  -> %s\n", deps->list[i]);
                if (install(root, deps->list[i]) < 0) {
                    fprintf(SYSERROR);
                    exit(errno);
                }
            }
        }

        printf("Installing package:\n");
        for (int i = 0; i < PACKAGE_MAX; i++) {
            char *match = NULL;
            char *package = NULL;

            if (!packages[i]) {
                break;
            }

            if ((match = find_package(packages[i])) == NULL) {
                fprintf(SYSERROR);
                exit(1);
            }

            if ((package = basename(match)) == NULL) {
                fprintf(stderr, "Unable to derive package name from package path:\n\t-> %s\n", match);
                exit(1);
            }

            // If the package was installed as a requirement of another dependency, skip it
            if (dep_seen(&deps, package)) {
                continue;
            }

            printf("  -> %s\n", package);
            if (install(root, packages[i]) < 0) {
                fprintf(SYSERROR);
                exit(errno);
            }
            free(match);
        }
        dep_free(&deps);
    }
    free_global_config();
    return 0;
}
