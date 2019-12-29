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
        NULL, NULL,
};

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

void mkprefix_usage(void) {
    printf("usage: mkprefix[bin|text] {output_file} {dir} {prefix ...}\n");
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

    if (strcmp(command, "mkprefixbin") == 0 || strcmp(command, "mkprefixtext") == 0) {
        char *outfile = argv[2];
        char *tree = argv[3];

        size_t prefix_start = 4;
        size_t prefixes = 0;
        for (size_t i = prefix_start; i < argc; i++) {
            prefixes = i;
        }

        // Check arguments
        if (!outfile) {
            fprintf(stderr, "error: missing output file name\n");
            mkprefix_usage();
            return -1;
        }
        if (!tree) {
            fprintf(stderr, "error: missing directory path\n");
            mkprefix_usage();
            return -1;
        }
        if (!prefixes) {
            fprintf(stderr, "error: missing prefix string(s)\n");
            mkprefix_usage();
            return -1;
        }

        char **prefix = (char **) calloc(prefixes + 1, sizeof(char *));
        if (!prefix) {
            perror("prefix array");
            fprintf(SYSERROR);
            return -1;
        }

        // Populate array of prefixes; reusing pointers from argv
        for (int i = 0; (i + prefix_start) < argc; i++) {
            prefix[i] = argv[(i + prefix_start)];
        }

        if (SPM_GLOBAL.verbose) {
            printf("Generating prefix manifest: %s\n", outfile);
        }

        if (strcmp(command, "mkprefixbin") == 0) {
            prefixes_write(outfile, PREFIX_WRITE_BIN, prefix, tree);
        } else if (strcmp(command, "mkprefixtext") == 0) {
            prefixes_write(outfile, PREFIX_WRITE_TEXT, prefix, tree);
        }
    }
    return 0;
}