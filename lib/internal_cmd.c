/**
 * @file internal_cmd.c
 */
#include "spm.h"

/**
 * List of valid internal commands
 */
static char *internal_commands[] = {
        "mkprefixbin", "generate prefix manifest (binary)",
        "mkprefixtext", "generate prefix manifest (text)",
        "mkmanifest", "generate package repository manifest",
        "mkruntime", "emit runtime environment (stdout)",
        "mirror_clone", "mirror a mirror",
        "rpath_set", "modify binary RPATH",
        "rpath_autoset", "determine nearest lib directory and set RPATH",
        "get_package_ext", "show the default archive extension",
        "get_sys_target", "show this system's arch/platform",
        "check_rt_env", "check the integrity of the calling runtime environment",
        NULL, NULL,
};

/**
 *
 */
void mkprefix_interface_usage(void) {
    printf("usage: mkprefix[bin|text] {output_file} {dir} {prefix ...}\n");
}

/**
 * Create prefix manifests from the CLI
 * @param argc
 * @param argv
 * @return return value of `prefixes_write`
 */
int mkprefix_interface(int argc, char **argv) {
    char *command = argv[0];
    char *outfile = argv[1];
    char *tree = argv[2];

    size_t prefix_start = 3;
    size_t prefixes = 0;
    for (size_t i = prefix_start; i < (size_t) argc; i++) {
        prefixes = i;
    }

    // Check arguments
    if (!outfile) {
        fprintf(stderr, "error: missing output file name\n");
        mkprefix_interface_usage();
        return -1;
    }
    if (!tree) {
        fprintf(stderr, "error: missing directory path\n");
        mkprefix_interface_usage();
        return -1;
    }
    if (!prefixes) {
        fprintf(stderr, "error: missing prefix string(s)\n");
        mkprefix_interface_usage();
        return -1;
    }

    char **prefix = (char **) calloc(prefixes + 1, sizeof(char *));
    if (!prefix) {
        perror("prefix array");
        fprintf(SYSERROR);
        return -1;
    }

    // Populate array of prefixes; reusing pointers from argv
    for (size_t i = 0; (i + prefix_start) < (size_t) argc; i++) {
        prefix[i] = argv[(i + prefix_start)];
    }

    if (SPM_GLOBAL.verbose) {
        printf("Generating prefix manifest: %s\n", outfile);
    }

    int result = 0;
    if (strcmp(command, "mkprefixbin") == 0) {
        result = prefixes_write(outfile, PREFIX_WRITE_BIN, prefix, tree);
    } else if (strcmp(command, "mkprefixtext") == 0) {
        result = prefixes_write(outfile, PREFIX_WRITE_TEXT, prefix, tree);
    }
    return result;
}

/**
 *
 */
void mkmanifest_interface_usage(void) {
    printf("usage: mkmanifest [-p target ...] [package_dir ...]\n");
}

/**
 * Generate a named package manifest
 * @param argc
 * @param argv
 * @return value of `manifest_write`
 */
int mkmanifest_interface(int argc, char **argv) {
    Manifest *manifest = NULL;
    int result = 0;
    char *pkgdir = NULL;
    char *path = NULL;
    char *target = NULL;
    StrList *paths = NULL;
    StrList *targets = NULL;

    if (argc < 2) {
        mkmanifest_interface_usage();
        return -1;
    }

    paths = strlist_init();
    targets = strlist_init();

    char *pth = NULL;
    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--platform") == 0) {
            i++;
            strlist_append(targets, argv[i]);
            continue;
        }

        pth = expandpath(argv[i]);
        if (pth == NULL || exists(pth) != 0) {
            fprintf(stderr, "%s does not exist\n", argv[i]);
            return -2;
        }

        strlist_append(paths, pth);
        free(pth);
    }

    size_t target_count = strlist_count(targets);
    size_t path_count = strlist_count(paths);

    if (target_count < 1) {
        strlist_append(targets, SPM_GLOBAL.repo_target);
        target_count = strlist_count(targets);
    }

    for (size_t t = 0; t < target_count; t++) {
        target = strlist_item(targets, t);
        for (size_t i = 0; i < path_count; i++) {
            path = strlist_item(paths, i);

            if (endswith(path, target)) {
                pkgdir = strdup(path);
            } else {
                pkgdir = join((char *[]) {path, target, NULL}, DIRSEPS);
            }

            if (SPM_GLOBAL.verbose) {
                printf("Indexing: %s\n", pkgdir);
            }

            if (exists(pkgdir) != 0) {
                fprintf(stderr, "WARNING: target directory does not exist: '%s'\n", pkgdir);
                continue;
            }

            manifest = manifest_from(pkgdir);
            if (manifest == NULL) {
                fprintf(stderr, "No packages\n");
                return -4;
            }

            result = manifest_write(manifest, pkgdir);
            if (result != 0) {
                fprintf(stderr, "ERROR:  while writing manifest data: '%s'\n", pkgdir);
                manifest_free(manifest);
                return -5;
            }
            free(pkgdir);
        }
    }

    if (targets != NULL)
        strlist_free(targets);

    if (paths != NULL)
        strlist_free(paths);

    if (manifest != NULL)
        manifest_free(manifest);

    return result;
}

/**
 *
 */
void mkruntime_interface_usage(void) {
    printf("usage: mkruntime {root_dir}\n");
}

/**
 * Generate a "key=value" string. If `_str` exists in the environment return a
 * a string like:
 *
 * ~~~{.c}
 * the_key=its_value
 * ~~~
 *
 * However, when `_str` does not exist in the environment it creates an empty
 * variable using setenv() and returns:
 *
 * ~~~{.c}
 * the_key=
 * ~~~
 *
 * @param _str a shell environment variable
 * @return `key=value` string
 */
static char *getenv_pair(const char *_str) {
    char *str = strdup(_str);
    char *result = NULL;

    if (getenv(str) == NULL) {
        if (setenv(str, "", 1) < 0) {
            perror("setenv failed");
            return NULL;
        }
    }
    result = join((char *[]){str, getenv(str), NULL}, "=");
    free(str);
    return result;
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int mkruntime_interface(int argc, char **argv) {
    if (argc < 2) {
        mkruntime_interface_usage();
        return -1;
    }

    // Environment variables listed here should also be referenced by the runtime_set calls below.
    // Do not needlessly append to this array.
    char *passthrough_runtime[] = {
            getenv_pair("PATH"),
            getenv_pair("MANPATH"),
            getenv_pair("PKG_CONFIG_PATH"),
            getenv_pair("ACLOCAL_PATH"),
            getenv_pair("CFLAGS"),
            getenv_pair("CPPFLAGS"),
            getenv_pair("CXXFLAGS"),
            getenv_pair("LDFLAGS"),
            NULL,
    };

    RuntimeEnv *rt = runtime_copy(passthrough_runtime);
    if (rt == NULL) {
        return -1;
    }

    char *root = argv[1];
    SPM_Hierarchy *fs = spm_hierarchy_init(root);
    char *spm_pkgconfigdir = join((char *[]) {fs->libdir, "pkgconfig", NULL}, DIRSEPS);

    runtime_set(rt, "SPM_ROOT", root);
    runtime_set(rt, "SPM_BIN", fs->bindir);
    runtime_set(rt, "SPM_INCLUDE", fs->includedir);
    runtime_set(rt, "SPM_LIB", fs->libdir);
    runtime_set(rt, "SPM_LIB64", "${SPM_LIB}64");
    runtime_set(rt, "SPM_DATA", fs->datadir);
    runtime_set(rt, "SPM_MAN", fs->mandir);
    runtime_set(rt, "SPM_LOCALSTATE", fs->localstatedir);
    runtime_set(rt, "SPM_PKGCONFIG", spm_pkgconfigdir);
#if OS_DARWIN
    runtime_set(rt, "SPM_PKGCONFIG", "${SPM_PKGCONFIG}:${SPM_DATA}/pkgconfig");
#elif OS_LINUX
    runtime_set(rt, "SPM_PKGCONFIG", "${SPM_PKGCONFIG}:${SPM_LIB64}/pkgconfig:${SPM_DATA}/pkgconfig");
#endif
    runtime_set(rt, "SPM_META_DEPENDS", SPM_META_DEPENDS);
    runtime_set(rt, "SPM_META_PREFIX_BIN", SPM_META_PREFIX_BIN);
    runtime_set(rt, "SPM_META_PREFIX_TEXT", SPM_META_PREFIX_TEXT);
    runtime_set(rt, "SPM_META_DESCRIPTOR", SPM_META_DESCRIPTOR);
    runtime_set(rt, "SPM_META_FILELIST", SPM_META_FILELIST);
    runtime_set(rt, "SPM_META_PREFIX_PLACEHOLDER", SPM_META_PREFIX_PLACEHOLDER);

    runtime_set(rt, "PATH", "$SPM_BIN:$$PATH");
    runtime_set(rt, "MANPATH", "$SPM_MAN:$$MANPATH");
    runtime_set(rt, "PKG_CONFIG_PATH", "$SPM_PKGCONFIG:$$PKG_CONFIG_PATH");
    runtime_set(rt, "ACLOCAL_PATH", "${SPM_DATA}/aclocal:$$ACLOCAL_PATH");

    char *spm_ccpath = join((char *[]) {fs->bindir, "gcc", NULL}, DIRSEPS);
    char *spm_cxxpath = join((char *[]) {fs->bindir, "g++", NULL}, DIRSEPS);
    char *spm_fcpath = join((char *[]) {fs->bindir, "gfortran", NULL}, DIRSEPS);

    if (exists(spm_ccpath) == 0) {
        runtime_set(rt, "CC", "$SPM_BIN/gcc");
    }

    if (exists(spm_cxxpath) == 0) {
        runtime_set(rt, "CXX", "$SPM_BIN/g++");
    }

    if (exists(spm_fcpath) == 0) {
        runtime_set(rt, "FC", "$SPM_BIN/gfortran");
    }

    runtime_set(rt, "CFLAGS", "-I$SPM_INCLUDE");
    runtime_set(rt, "CPPFLAGS", "-I$SPM_INCLUDE");
    runtime_set(rt, "CXXFLAGS", "-I$SPM_INCLUDE");
#if OS_DARWIN
    // For now `reloc` can fix up the LC_ID_DYLIB on its own without install_name_tool
    runtime_set(rt, "LDFLAGS", "-Wl,-rpath,$SPM_LIB -L$SPM_LIB");
#elif OS_LINUX
    runtime_set(rt, "LDFLAGS", "-Wl,-rpath=$SPM_LIB:$SPM_LIB64 -L$SPM_LIB -L$SPM_LIB64");
#else
    // TODO: Windows?
#endif
    runtime_export(rt, NULL);
    runtime_free(rt);

    for (size_t i = 0; passthrough_runtime[i] != NULL; i++) {
        free(passthrough_runtime[i]);
    }

    free(spm_pkgconfigdir);
    free(spm_ccpath);
    free(spm_cxxpath);
    free(spm_fcpath);
    spm_hierarchy_free(fs);
    return 0;
}

/**
 *
 */
void mirror_clone_interface_usage(void) {
    printf("usage: mirror_clone {url} {output_dir}\n");
}

/**
 * Mirror packages referenced by a remote manifest
 * @param argc
 * @param argv
 * @return value of `manifest_write`
 */
int mirror_clone_interface(int argc, char **argv) {
    if (argc < 3) {
        mirror_clone_interface_usage();
        return -1;
    }
    char *url = argv[1];
    char *_path = argv[2];
    char *path;
    StrList *targets;
    char *target_file;
    size_t target_count;
    int no_mirroring;

    path = NULL;
    no_mirroring = 0;
    targets = strlist_init();
    target_file = join((char *[]) {url, "spm_mirror.targets", NULL}, DIRSEPS);

    if (strlist_append_file(targets, target_file, NULL) != 0) {
        no_mirroring = 1;
    }

    if (targets != NULL) {
        target_count = strlist_count(targets);
        if (target_count == 0) {
            no_mirroring = 1;
        }
    }

    if (no_mirroring) {
        fprintf(stderr, "Mirroring is disabled by the server.");
        return 1;
    }

    for (size_t tgt = 0; tgt < target_count; tgt++) {
        char *manifest_location;
        char *target;

        target = strlist_item(targets, tgt);
        strip(target);

        manifest_location = join((char *[]) {url, target, NULL}, DIRSEPS);
        Manifest *manifest = manifest_read(manifest_location);
        if (manifest == NULL) {
            return -2;
        }

        path = join((char *[]) {_path, target, NULL}, DIRSEPS);
        mirror_clone(manifest, path);
        manifest_free(manifest);
    }

    return 0;
}
/**
 *
 */
void rpath_set_interface_usage(void) {
    printf("usage: rpath_set {file} {rpath}\n");
}

/**
 * Set a RPATH from the CLI
 * @param argc
 * @param argv
 * @return return value of `rpath_set`
 */
int rpath_set_interface(int argc, char **argv) {
    if (argc < 3) {
        rpath_set_interface_usage();
        return -1;
    }
    char *filename = argv[1];
    char *rpath = argv[2];
    int result = rpath_set(filename, rpath);
    if (result < 0) {
        fprintf(SYSERROR);
    }
    return result;
}

/**
 *
 */
void rpath_autoset_interface_usage(void) {
    printf("usage: rpath_autoset {file} {topdir} {destroot}\n");
}

/**
 * Set a RPATH automatically from the CLI
 * @param argc
 * @param argv
 * @return return value of `rpath_autoset`
 */
int rpath_autoset_interface(int argc, char **argv) {
    if (argc < 4) {
        rpath_autoset_interface_usage();
        return -1;
    }
    char *filename = argv[1];
    const char *topdir = argv[2];
    const char *destroot = argv[3];

    if (exists(filename) != 0) {
        perror(filename);
        return -1;
    }

    if (exists(topdir) != 0) {
        perror(topdir);
        return -1;
    }

    FSTree *libs = rpath_libraries_available(topdir);
    int result = rpath_autoset(filename, libs, destroot);

    if (result < 0) {
        fprintf(SYSERROR);
    }

    return result;
}

/**
 * Dump the default package extension for SPM archives to `stdout`
 * @return
 */
int get_package_ext_interface(void) {
    puts(SPM_PACKAGE_EXTENSION);
    return 0;
}

/**
 * Dump the system arch/platform (i.e. Linux/x86_64)
 * @return
 */
int get_sys_target_interface(void) {
    puts(SPM_GLOBAL.repo_target);
    return 0;
}
/**
 * Execute builtin runtime check.
 *
 * On failure this function will EXIT the program with a non-zero value
 *
 * @return
 */
int check_runtime_environment_interface(void) {
    check_runtime_environment();
    return 0;
}

/**
 * Show a listing of valid internal commands
 */
void internal_command_list(void) {
    printf("possible commands:\n");
    for (size_t i = 0; internal_commands[i] != NULL; i += 2) {
        printf("  %-20s - %-20s\n", internal_commands[i], internal_commands[i + 1]);
    }
}

/**
 * Execute an internal command
 * @param argc
 * @param argv
 * @return success=0, failure=1, error=-1
 */
int internal_cmd(int argc, char **argv) {
    int command_valid = 0;
    char *command = argv[1];
    if (argc < 2) {
        internal_command_list();
        return 1;
    }

    for (int i = 0; internal_commands[i] != NULL; i++) {
        if (strcmp(internal_commands[i], command) == 0) {
            command_valid = 1;
            break;
        }
    }

    if (!command_valid) {
        fprintf(stderr, "error: '%s' is not a valid command\n", command);
        internal_command_list();
        return 1;
    }

    // Strip the first argument (this level) before passing it along to the interface
    int arg_count = argc - 1;
    char **arg_array = &argv[1];

    // Normalize the argument counter when there's no arguments to speak of
    if (arg_count < 0) {
        arg_count = 0;
    }

    if (strcmp(command, "mkprefixbin") == 0 || strcmp(command, "mkprefixtext") == 0) {
        return mkprefix_interface(arg_count, arg_array);
    }
    else if (strcmp(command, "mkmanifest") == 0) {
        return mkmanifest_interface(arg_count, arg_array);
    }
    else if (strcmp(command, "mkruntime") == 0) {
        return mkruntime_interface(arg_count, arg_array);
    }
    else if (strcmp(command, "mirror_clone") == 0) {
        return mirror_clone_interface(arg_count, arg_array);
    }
    else if (strcmp(command, "rpath_set") == 0) {
        return rpath_set_interface(arg_count, arg_array);
    }
    else if (strcmp(command, "rpath_autoset") == 0) {
        return rpath_autoset_interface(arg_count, arg_array);
    }
    else if (strcmp(command, "get_package_ext") == 0) {
        return get_package_ext_interface();
    }
    else if (strcmp(command, "get_sys_target") == 0) {
        return get_sys_target_interface();
    }
    else if (strcmp(command, "check_rt_env") == 0) {
        return check_runtime_environment_interface();
    }
    return 0;
}
