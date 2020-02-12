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

    int result;
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
    printf("usage: mkmanifest [package_dir] [output_dir]\n");
}

/**
 * Generate a named package manifest
 * @param argc
 * @param argv
 * @return value of `manifest_write`
 */
int mkmanifest_interface(int argc, char **argv) {
    if (argc < 1) {
        mkmanifest_interface_usage();
        return -1;
    }
    Manifest *manifest = NULL;
    int result = 0;
    char *pkgdir = NULL;
    char path[PATH_MAX];
    memset(path, '\0', PATH_MAX);

    if (argc == 2) {
        pkgdir = argv[1];
    }
    else {
        pkgdir = SPM_GLOBAL.package_dir;
    }

    if (argc > 2) {
        strcpy(path, argv[2]);
    }
    else {
        strcpy(path, SPM_MANIFEST_FILENAME);
    }

    if (exists(pkgdir) != 0) {
        return -1;
    }

    manifest = manifest_from(pkgdir);
    if (manifest == NULL) {
        return -2;
    }

    result = manifest_write(manifest, path);
    if (result != 0) {
        manifest_free(manifest);
        return -3;
    }

    manifest_free(manifest);
    return result;
}

/**
 *
 */
void mkruntime_interface_usage(void) {
    printf("usage: mkruntime {root_dir}");
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

    RuntimeEnv *rt = runtime_copy(__environ);
    if (rt == NULL) {
        return -1;
    }

    char *root = argv[1];
    char *spm_binpath = join((char *[]) {root, "bin", NULL}, DIRSEPS);
    char *spm_includepath = join((char *[]) {root, "include", NULL}, DIRSEPS);
    char *spm_libpath = join((char *[]) {root, "lib", NULL}, DIRSEPS);
    char *spm_datapath = join((char *[]) {root, "share", NULL}, DIRSEPS);
    char *spm_manpath = join((char *[]) {spm_datapath, "man", NULL}, DIRSEPS);

    runtime_set(rt, "SPM_BIN", spm_binpath);
    runtime_set(rt, "SPM_INCLUDE", spm_includepath);
    runtime_set(rt, "SPM_LIB", spm_libpath);
    runtime_set(rt, "SPM_DATA", spm_datapath);
    runtime_set(rt, "SPM_MAN", spm_manpath);
    runtime_set(rt, "PATH", "$SPM_BIN:$PATH");
    runtime_set(rt, "MANPATH", "$SPM_MAN:$MANPATH");

    char *spm_ccpath = join((char *[]) {spm_binpath, "gcc"}, DIRSEPS);
    if (exists(spm_ccpath) == 0) {
        runtime_set(rt, "CC", "$SPM_BIN/gcc");
    }

    runtime_set(rt, "CFLAGS", "-I$SPM_INCLUDE $CFLAGS");
    runtime_set(rt, "LDFLAGS", "-Wl,-rpath=$SPM_LIB:$${ORIGIN}/../lib -L$SPM_LIB $LDFLAGS");
    runtime_export(rt, NULL);
    runtime_free(rt);

    free(spm_binpath);
    free(spm_includepath);
    free(spm_libpath);
    free(spm_datapath);
    free(spm_manpath);
    free(spm_ccpath);
    return 0;
}

/**
 *
 */
void mirror_clone_interface_usage(void) {
    printf("usage: mirror_clone {url} {output_dir}");
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
    char *path = argv[2];

    Manifest *manifest = manifest_read(url);
    if (manifest == NULL) {
        return -2;
    }

    mirror_clone(manifest, path);
    manifest_free(manifest);
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
    printf("usage: rpath_autoset {file}\n");
}

/**
 * Set a RPATH automatically from the CLI
 * @param argc
 * @param argv
 * @return return value of `rpath_autoset`
 */
int rpath_autoset_interface(int argc, char **argv) {
    if (argc < 2) {
        rpath_autoset_interface_usage();
        return -1;
    }
    char *filename = argv[1];
    int result = rpath_autoset(filename);
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
    for (size_t i = 0; internal_commands[i] != NULL; i++) { // TODO: fix double increment warning (i++ becomes i+2?)
        printf("  %-20s - %-20s\n", internal_commands[i], internal_commands[i + 1]);
        i++;
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
    else if (strcmp(command, "check_rt_env") == 0) {
        return check_runtime_environment_interface();
    }
    return 0;
}
