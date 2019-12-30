/**
 * @file relocation.c
 */
#include "spm.h"

/**
 * Replace all occurrences of `spattern` with `sreplacement` in `data`
 *
 * ~~~{.c}
 * char *str = (char *)calloc(100, sizeof(char));
 * strcpy(str, "This are a test.");
 * replace_text(str, "are", "is");
 * // str is: "This is a test."
 * free(str);
 * ~~~
 *
 * @param data string to modify
 * @param spattern string value to replace
 * @param sreplacement replacement string value
 * @return success=0, error=-1
 */
int replace_text(char *data, const char *spattern, const char *sreplacement) {
    char *tmp = data;
    size_t data_len = strlen(data);
    size_t spattern_len = strlen(spattern);
    size_t sreplacement_len = strlen(sreplacement);

    if (sreplacement_len > spattern_len) {
        fprintf(stderr, "replacement string too long\n");
        return -1;
    }

    while (*tmp != '\0') {
        if (strncmp(tmp, spattern, spattern_len) == 0) {
            memmove(tmp, sreplacement, sreplacement_len);
            memmove(tmp + sreplacement_len, tmp + spattern_len, data_len - spattern_len);
        }
        tmp++;
    }
    return 0;
}

/**
 * Replace all occurrences of `oldstr` in file `path` with `newstr`
 * @param filename file to modify
 * @param oldstr string to replace
 * @param newstr replacement string
 * @return success=0, failure=-1, or value of `ferror()`
 */
int file_replace_text(char *filename, const char *spattern, const char *sreplacement) {
    char data[BUFSIZ];
    char tempfile[PATH_MAX];
    FILE *fp = NULL;
    if ((fp = fopen(filename, "r")) == NULL) {
        perror(filename);
        return -1;
    }

    sprintf(tempfile, "%s.spmfrt", filename);
    FILE *tfp = NULL;
    if ((tfp = fopen(tempfile, "w+")) == NULL) {
        fclose(fp);
        perror(tempfile);
        return -1;
    }

    // Zero the data buffer
    memset(data, '\0', BUFSIZ);
    while(fgets(data, BUFSIZ, fp) != NULL) {
        replace_text(data, spattern, sreplacement);
        fprintf(tfp, "%s", data);
    }
    fclose(fp);
    rewind(tfp);

    // Truncate the original file
    if ((fp = fopen(filename, "w+")) == NULL) {
        perror(filename);
        return -1;
    }
    // Zero the data buffer once more
    memset(data, '\0', BUFSIZ);
    // Dump the contents of the temporary file into the original file
    while(fgets(data, BUFSIZ, tfp) != NULL) {
        fprintf(fp, "%s", data);
    }
    fclose(fp);
    fclose(tfp);

    // Remove temporary file
    unlink(tempfile);
    return 0;
}

/**
 * Free memory allocated by `prefixes_read` function
 * @param entry array of RelocationEntry
 */
void prefixes_free(RelocationEntry **entry) {
    if (!entry) {
        return;
    }
    for (int i = 0; entry[i] != NULL; i++) {
        if (entry[i]->prefix) free(entry[i]->prefix);
        if (entry[i]->path) free(entry[i]->path);
        if (entry[i]) free(entry[i]);
    }
    free(entry);
}

/**
 * Parse a prefix file
 *
 * The file format is as follows:
 *
 * ~~~
 * #prefix
 * path
 * #prefix
 * path
 * #...N
 * ...N
 * ~~~
 * @param filename path to prefix manifest
 * @return success=array of RelocationEntry, failure=NULL
 */
RelocationEntry **prefixes_read(const char *filename) {
    size_t record_count = 0;
    size_t parity = 0;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(SYSERROR);
        return NULL;
    }
    RelocationEntry **entry = NULL;
    char line[BUFSIZ];
    memset(line, '\0', BUFSIZ);

    while (fgets(line, BUFSIZ, fp) != NULL) {
        if (isempty(line)) {
            continue;
        }
        record_count++;
    }
    rewind(fp);

    // Initialize the relocation entry array
    if (record_count == 0) {
        return NULL;
    }

    parity = record_count % 2;
    if (parity != 0) {
        fprintf(stderr, "%s: records are not divisible by 2 (got: %zu %% 2 = %zu)\n", filename, record_count, parity);
        return NULL;
    }
    record_count /= 2;

    entry = (RelocationEntry **)calloc(record_count + 1, sizeof(RelocationEntry *));
    if (!entry) {
        return NULL;
    }
    for (size_t i = 0; i < record_count; i++) {
        entry[i] = (RelocationEntry *) calloc(1, sizeof(RelocationEntry));
        if (!entry[i]) {
            return NULL;
        }
    }

    int do_prefix = 0;
    int do_path = 0;
    size_t i = 0;
    while (fgets(line, BUFSIZ, fp) != NULL) {
        if (isempty(line)) {
            continue;
        }
        if (startswith(line, "#") == 0) {
            do_prefix = 1;
        }
        else {
            do_path = 1;
        }

        // Allocate a relocation record
        if (!entry[i]) {
            fclose(fp);
            return NULL;
        }


        if (do_prefix) {
            // Populate prefix data (a prefix starts with a #)
            entry[i]->prefix = (char *) calloc(strlen(line) + 1, sizeof(char));
            if (!entry[i]->prefix) {
                fclose(fp);
                return NULL;
            }
            strncpy(entry[i]->prefix, line, strlen(line));
            // Remove prefix delimiter and whitespace
            strchrdel(entry[i]->prefix, "#");
            entry[i]->prefix = strip(entry[i]->prefix);
            do_prefix = 0;
            continue;
        }

        else if (do_path) {
            // Populate path data
            entry[i]->path = (char *) calloc(strlen(line) + 1, sizeof(char));
            if (!entry[i]->path) {
                fclose(fp);
                return NULL;
            }
            strncpy(entry[i]->path, line, strlen(line));
            entry[i]->path = strip(entry[i]->path);
            do_path = 0;
        }
        i++;
    }
    fclose(fp);
    return entry;
}

/**
 * Scan `tree` for files containing `prefix`. Matches are recorded in `output_file` with the following format:
 *
 * ~~~
 * #prefix
 * path
 * #prefix
 * path
 * #...N
 * ...N
 * ~~~
 *
 * Example:
 * ~~~{.c}
 * char **prefixes = {"/usr", "/var", NULL};
 * prefixes_write("binary.manifest", PREFIX_WRITE_BIN, prefixes, "/usr/bin");
 * prefixes_write("text.manifest", PREFIX_WRITE_TEXT, prefixes, "/etc");
 * ~~~
 *
 * @param output_file file path to create
 * @param mode `PREFIX_WRITE_BIN`, `PREFIX_WRITE_TEXT`
 * @param prefix array of prefix strings
 * @param tree directory to scan
 * @return success=0, failure=1, error=-1
 */
int prefixes_write(const char *output_file, int mode, char **prefix, const char *tree) {
    FILE *fp = fopen(output_file, "w+");
    if (!fp) {
        perror(output_file);
        fprintf(SYSERROR);
        return -1;
    }

    FSTree *fsdata = fstree(tree);
    if (!fsdata) {
        fclose(fp);
        fprintf(SYSERROR);
        return -1;
    }
    for (size_t i = 0; i < fsdata->files_length; i++) {
        for (int p = 0; prefix[p] != NULL; p++) {
            if (find_in_file(fsdata->files[i], prefix[p]) == 0) {
                int proceed = 0;
                if (mode == PREFIX_WRITE_BIN) {
                    proceed = file_is_binary(fsdata->files[i]);
                } else if (mode == PREFIX_WRITE_TEXT) {
                    proceed = file_is_text(fsdata->files[i]);
                }

                if (!proceed) {
                    continue;
                }

                fprintf(fp, "#%s\n%s\n", prefix[p], fsdata->files[i]);
            }
        }
    }
    fclose(fp);
    return 0;
}

/**
 * Wrapper for `reloc` program. Replace text in binary data.
 * @param _filename
 * @param _oldstr
 * @param _newstr
 * @return
 */
int relocate(const char *_filename, const char *_oldstr, const char *_newstr) {
    int returncode;
    Process *proc = NULL;
    char *oldstr = strdup(_oldstr);
    char *newstr = strdup(_newstr);
    char *filename = strdup(_filename);
    char cmd[PATH_MAX];

    memset(cmd, '\0', sizeof(cmd));
    sprintf(cmd, "reloc \"%s\" \"%s\" \"%s\" \"%s\"", oldstr, newstr, filename, filename);

    // sanitize command
    strchrdel(cmd, "&;|");

    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        free(oldstr);
        free(newstr);
        free(filename);
        return -1;
    }

    returncode = proc->returncode;
    if (returncode != 0 && proc->output) {
        fprintf(stderr, proc->output);
    }

    shell_free(proc);
    free(oldstr);
    free(newstr);
    free(filename);
    return returncode;
}

