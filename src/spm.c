/**
 * SPM - Simple Package Manager
 * @file spm.c
 */
#include <errno.h>
#include "spm.h"

int RUNTIME_REMOVE = 0;
int RUNTIME_INSTALL = 0;
int RUNTIME_ROOTDIR = 0;
int RUNTIME_LIST = 0;
int RUNTIME_SEARCH = 0;

void usage(const char *program_name) {
    printf("usage: %s [-hVvBIrmMLS]\n"
           "  -h,  --help                show this help message\n"
           "  -V,  --version             show version\n"
           "  -v,  --verbose             show more information\n"
           "  -y   --yes                 do not prompt\n"
           "  -B,  --build               build package(s)\n"
           "  -I,  --install             install package(s)\n"
           "  -r,  --root                installation prefix (requires --install)\n"
           "  -m   --manifest            specify a package manifest to use\n"
           "  -M   --override-manifests  disable default package manifest location\n"
           "  -L,  --list                list available packages\n"
           "  -S,  --search              search for a package\n"
           "       --cmd                 execute an internal spm command\n"
           , program_name);
}

int main(int argc, char *argv[], char *arge[]) {
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

    char rootdir[PATH_MAX];
    StrList *packages = strlist_init();
    ManifestList *mf = NULL;
    char package_search_str[PATH_MAX];
    int override_manifests = 0;

    memset(rootdir, '\0', PATH_MAX);
    memset(package_search_str, '\0', PATH_MAX);

    if (argc < 2) {
        usage(program_name);
        exit(1);
    }

    mf = manifestlist_init();

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
            else if (strcmp(arg, "-y") == 0 || strcmp(arg, "--yes") == 0) {
                SPM_GLOBAL.prompt_user = 0;
            }
            else if (strcmp(arg, "-M") == 0 || strcmp(arg, "--override-manifests") == 0) {
                override_manifests = 1;
            }
            else if (strcmp(arg, "-m") == 0 || strcmp(arg, "--manifest") == 0) {
                if (arg_next == NULL) {
                    fprintf(stderr, "-m|--manifest requires a directory path\n");
                    usage(program_name);
                    exit(1);
                }
                manifestlist_append(mf, arg_next);
                i++;
            }
            else if (strcmp(arg, "--reindex") == 0) {
                Manifest *info = manifest_from(SPM_GLOBAL.package_dir);
                manifest_write(info, SPM_GLOBAL.package_manifest);
                manifest_free(info);
                exit(0);
            }
            else if (strcmp(arg, "--cmd") == 0) {
                int c = argc - i;
                char **a = &argv[i];
                int retval = internal_cmd(c, a);
                free_global_config();
                exit(retval);
            }
            else if (strcmp(arg, "-B") == 0 || strcmp(arg, "--build") == 0) {
                int c = argc - i;
                char **a = &argv[i];
                exit(build(c, a));
            }
            else if (strcmp(arg, "-L") == 0 || strcmp(arg, "--list") == 0) {
                RUNTIME_LIST = 1;
            }
            else if (strcmp(arg, "-S") == 0 || strcmp(arg, "--search") == 0) {
                RUNTIME_SEARCH = 1;
                if (arg_next == NULL) {
                    fprintf(stderr, "--search requires a package name\n");
                    usage(program_name);
                    exit(1);
                }
                strncpy(package_search_str, arg_next, strlen(arg_next));
                i++;
            }
            else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--root") == 0) {
                RUNTIME_ROOTDIR = 1;
                if (!arg_next) {
                    fprintf(stderr, "-r|--root requires a path\n");
                    usage(program_name);
                    exit(1);
                }
                strcpy(rootdir, arg_next);
                i++;
            }
            else if (strcmp(arg, "-R") == 0 || strcmp(arg, "--remove") == 0) {
                RUNTIME_REMOVE = 1;
                for (int p = 0; i < argc; p++) {
                    i++;
                    char *next = argv[i];
                    if (next == NULL || (startswith(next, "-") == 0 || startswith(next, "--") == 0)) {
                        i--;
                        break;
                    }
                    if ((argc - i) == 0) {
                        fprintf(stderr, "-R|--remove requires at least one package\n");
                        usage(program_name);
                        exit(1);
                    }
                    strlist_append(packages, argv[i]);
                }
            }
            else if (strcmp(arg, "-I") == 0 || strcmp(arg, "--install") == 0) {
                RUNTIME_INSTALL = 1;
                for (int p = 0; i < argc; p++) {
                    i++;
                    char *next = argv[i];
                    if (next == NULL || (startswith(next, "-") == 0 || startswith(next, "--") == 0)) {
                        i--;
                        break;
                    }
                    if ((argc - i) == 0) {
                        fprintf(stderr, "-I|--install requires at least one package\n");
                        usage(program_name);
                        exit(1);
                    }
                    strlist_append(packages, next);
                }
            }
        }
        else {
            printf("Unknown option: %s\n", arg);
            usage(program_name);
            exit(1);
        }
    }

    if (override_manifests == 0) {
        // Place the default package location at the bottom of the list
        manifestlist_append(mf, SPM_GLOBAL.package_dir);
    }

    // Dump configuration
    if (SPM_GLOBAL.verbose) {
        show_global_config();
    }

    if (!RUNTIME_ROOTDIR && RUNTIME_INSTALL) {
        fprintf(stderr, "-r|--root requires -I|--install\n");
        usage(program_name);
        exit(1);
    } else if (!RUNTIME_ROOTDIR && RUNTIME_REMOVE) {
        fprintf(stderr, "-r|--root requires -R|--remove\n");
        usage(program_name);
        exit(1);
    }

    if (isempty(rootdir)) {
        sprintf(rootdir, "%s%c%s", getenv("HOME"), DIRSEP, "spm_root");
    }

    SPM_Hierarchy *rootfs = spm_hierarchy_init(rootdir);

    // Construct installation runtime environment
    RuntimeEnv *rt = runtime_copy(arge);
    // TODO: Move environment allocation out of (above) this loop if possible
    // TODO: replace variables below with SPM_Hierarchy, and write some control functions

    runtime_set(rt, "SPM_BIN", rootfs->bindir);
    runtime_set(rt, "SPM_INCLUDE", rootfs->includedir);
    runtime_set(rt, "SPM_LIB", rootfs->libdir);
    runtime_set(rt, "SPM_DATA", rootfs->datadir);
    runtime_set(rt, "SPM_MAN", rootfs->mandir);
    runtime_set(rt, "SPM_LOCALSTATE", rootfs->localstatedir);
    runtime_apply(rt);

    if (RUNTIME_REMOVE) {
        int status_remove = 0;
        if ((status_remove = spm_do_purge(rootfs, packages)) == -1) {
            exit(1);
        }
        else if (status_remove == -2) {
            // user said no when asked to proceed
            exit(2);
        }

    }

    if (RUNTIME_INSTALL) {
        int status_install = 0;
        if ((status_install = spm_do_install(rootfs, mf, packages)) == -1) {
            // failed to create temporary destination root
            exit(1);
        }
        else if (status_install == -2) {
            // user said no when asked to proceed
            exit(2);
        }
    }

    if (RUNTIME_SEARCH || RUNTIME_LIST) {
        char name[255];
        char op[25];
        char ver[255];
        memset(name, '\0', sizeof(name));
        memset(op, '\0', sizeof(op));
        memset(ver, '\0', sizeof(ver));

        // Parse the argument string
        int c = 0;

        // Populate name
        for (int j = 0; package_search_str[c] != '\0'; j++, c++) {
            if (isrelational(package_search_str[c])) {
                break;
            }
            name[j] = package_search_str[c];
        }

        if (RUNTIME_SEARCH) {
            // Populate op
            for (int j = 0; package_search_str[c] != '\0'; j++, c++) {
                if (!isrelational(package_search_str[c])) {
                    break;
                }
                op[j] = package_search_str[c];
            }

            if (strlen(op)) {
                // Populate version
                for (int j = 0; package_search_str[c] != '\0'; j++, c++) {
                    ver[j] = package_search_str[c];
                }
            } else {
                // No operator, so find all versions instead
                strcpy(op, ">=");
                ver[0] = '0';
            }

        }

        int banner_size = 79;
        putchar('#');
        print_banner("-", banner_size);
        printf("# %-20s %-10s %-10s %-10s %-20s\n", "name", "version", "revision", "size", "origin");
        putchar('#');
        print_banner("-", banner_size);

        for (size_t m = 0; m < manifestlist_count(mf); m++) {
            Manifest *info = manifestlist_item(mf, m);
            if (RUNTIME_SEARCH) {
                ManifestPackage **package = find_by_spec(info, name, op, ver);
                for (int p = 0; package[p] != NULL; p++) {
                    char *package_hsize = human_readable_size(package[p]->size);
                    printf("  %-20s %-10s %-10s %-10s %20s\n", package[p]->name, package[p]->version, package[p]->revision,
                           package_hsize, info->packages[p]->origin);
                    free(package_hsize);
                }
            } else if (RUNTIME_LIST) {
                for (size_t p = 0; p < info->records; p++) {
                    char *package_hsize = human_readable_size(info->packages[p]->size);
                    printf("  %-20s %-10s %-10s %-10s %20s\n", info->packages[p]->name, info->packages[p]->version,
                           info->packages[p]->revision, package_hsize, info->packages[p]->origin);
                    free(package_hsize);
                }
            }
        }
    }

    runtime_free(rt);
    free_global_config();
    strlist_free(packages);
    manifestlist_free(mf);
    spm_hierarchy_free(rootfs);
    return 0;
}
