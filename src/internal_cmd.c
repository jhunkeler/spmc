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
        "mirror_clone", "mirror a mirror",
        "rpath_set", "modify binary RPATH",
        "rpath_autoset", "determine nearest lib directory and set RPATH",
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
    else if (strcmp(command, "mirror_clone") == 0) {
        return mirror_clone_interface(arg_count, arg_array);
    }
    else if (strcmp(command, "rpath_set") == 0) {
        return rpath_set_interface(arg_count, arg_array);
    }
    else if (strcmp(command, "rpath_autoset") == 0) {
        return rpath_autoset_interface(arg_count, arg_array);
    }
    return 0;
}