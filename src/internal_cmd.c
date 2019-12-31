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
    printf("usage: rpath_autoset {file} {rpath}\n");
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
    for (int i = 0; internal_commands[i] != NULL; i++) {
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
    else if (strcmp(command, "rpath_set") == 0) {
        return rpath_set_interface(arg_count, arg_array);
    }
    return 0;
}