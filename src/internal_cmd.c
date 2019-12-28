#include "spm.h"

static char *internal_commands[] = {
        "mkprefixbin", "generate prefix manifest (binary)",
        "mkprefixtext", "generate prefix manifest (text)",
        NULL, NULL,
};

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

        int prefix_start = 4;
        int prefixes;
        for (int i = prefix_start; i < argc; i++) {
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
            fprintf(stderr, "error: missing prefix string(s) (%d, %d)\n", prefix_start, prefixes);
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