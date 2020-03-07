/**
 * @file relocation.c
 */
#include "spm.h"

const char *METADATA_FILES[] = {
        SPM_META_DEPENDS,
        SPM_META_PREFIX_BIN,
        SPM_META_PREFIX_TEXT,
        SPM_META_DESCRIPTOR,
        SPM_META_FILELIST,
        NULL,
};

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
        fprintf(stderr, "replacement string too long: %zu > %zu\n  '%s'\n  '%s'\n", sreplacement_len, spattern_len, sreplacement, spattern);
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
    char *lptr = line;
    memset(lptr, '\0', BUFSIZ);

    while (fgets(lptr, BUFSIZ, fp) != NULL) {
        if (isempty(lptr)) {
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
    while (fgets(lptr, BUFSIZ, fp) != NULL) {
        if (isempty(lptr)) {
            continue;
        }
        if (startswith(lptr, "#")) {
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
            entry[i]->prefix = (char *) calloc(strlen(lptr) + 1, sizeof(char));
            if (!entry[i]->prefix) {
                fclose(fp);
                return NULL;
            }
            strncpy(entry[i]->prefix, lptr, strlen(lptr));
            // Remove prefix delimiter and whitespace
            strchrdel(entry[i]->prefix, "#");
            entry[i]->prefix = strip(entry[i]->prefix);
            do_prefix = 0;
            continue;
        }

        else if (do_path) {
            // Populate path data
            entry[i]->path = (char *) calloc(strlen(lptr) + 1, sizeof(char));
            if (!entry[i]->path) {
                fclose(fp);
                return NULL;
            }
            strncpy(entry[i]->path, lptr, strlen(lptr));
            entry[i]->path = strip(entry[i]->path);
            do_path = 0;
        }
        i++;
    }
    fclose(fp);
    return entry;
}

/**
 * Determine if `filename` is a SPM metadata file
 *
 * Example:
 *
 * ~~~{.c}
 * #include "spm.h"
 *
 * int main() {
 *     if (file_is_metadata(".SPM_DEPENDS")) {
 *         // file is metadata
 *     } else {
 *         // file is not metadata
 *     }
 * }
 * ~~~
 *
 * @param filename
 * @return 0=no, 1=yes
 */
int file_is_metadata(const char *filename) {
    for (size_t i = 0; METADATA_FILES[i] != NULL; i++) {
        if (strstr(filename, METADATA_FILES[i]) != NULL) {
            return 1;
        }
    }
    return 0;
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

    char *cwd = getcwd(NULL, PATH_MAX);
    chdir(tree);
    {
        FSTree *fsdata = fstree(".", NULL, SPM_FSTREE_FLT_RELATIVE);
        if (!fsdata) {
            fclose(fp);
            fprintf(SYSERROR);
            return -1;
        }
        for (size_t i = 0; i < fsdata->files_length; i++) {
            if (file_is_metadata(fsdata->files[i])) {
                continue;
            }
            for (int p = 0; prefix[p] != NULL; p++) {
                if (find_in_file(fsdata->files[i], prefix[p]) == 0) {
                    int proceed = 0;
                    if (mode == PREFIX_WRITE_BIN) {
                        proceed = file_is_binary(fsdata->files[i]);
                    } else if (mode == PREFIX_WRITE_TEXT) {
                        proceed = file_is_text(fsdata->files[i]);
                    }

                    // file_is_* functions return NULL when they encounter anything but a regular file
                    if (!proceed) {
                        continue;
                    }
                    // Record in file
                    fprintf(fp, "#%s\n%s\n", prefix[p], fsdata->files[i]);
                }
            }
        }
    } chdir(cwd);
    free(cwd);
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

    // sanitize command
    strchrdel(oldstr, SHELL_INVALID);
    strchrdel(newstr, SHELL_INVALID);
    strchrdel(filename, SHELL_INVALID);

    memset(cmd, '\0', sizeof(cmd));
    sprintf(cmd, "reloc \"%s\" \"%s\" \"%s\" \"%s\" 2>&1", oldstr, newstr, filename, filename);

    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        free(oldstr);
        free(newstr);
        free(filename);
        return -1;
    }

    returncode = proc->returncode;
    if (returncode != 0 && proc->output) {
        fprintf(stderr, "%s\n", proc->output);
    }

    shell_free(proc);
    free(oldstr);
    free(newstr);
    free(filename);
    return returncode;
}

/**
 * Parse package metadata and set `baseroot` binaries/text to point to `destroot`.
 * `baseroot` should be a temporary directory because its contents are modified
 *
 * @param destroot
 * @param baseroot
 */
void relocate_root(const char *destroot, const char *baseroot) {
    RelocationEntry **b_record = NULL;
    RelocationEntry **t_record = NULL;
    char cwd[PATH_MAX];

    getcwd(cwd, sizeof(cwd));
    chdir(baseroot);
    {
        FSTree *libs = rpath_libraries_available(".");
        // Rewrite binary prefixes
        b_record = prefixes_read(SPM_META_PREFIX_BIN);
        if (b_record) {
            for (int i = 0; b_record[i] != NULL; i++) {
                if (file_is_binexec(b_record[i]->path)) {
                    if (SPM_GLOBAL.verbose) {
                        printf("Relocate RPATH: %s\n", b_record[i]->path);
                    }
                    rpath_autoset(b_record[i]->path, libs);
                }
                if (SPM_GLOBAL.verbose) {
                    printf("Relocate DATA : %s\n", b_record[i]->path);
                }
                relocate(b_record[i]->path, b_record[i]->prefix, destroot);
            }
        }

        // Rewrite text prefixes
        t_record = prefixes_read(SPM_META_PREFIX_TEXT);
        if (t_record) {
            for (int i = 0; t_record[i] != NULL; i++) {
                if (SPM_GLOBAL.verbose) {
                    printf("Relocate TEXT : %s (%s -> %s)\n", t_record[i]->path, t_record[i]->prefix, destroot);
                }
                file_replace_text(t_record[i]->path, t_record[i]->prefix, destroot);
            }
        }

        prefixes_free(b_record);
        prefixes_free(t_record);
    }
    chdir(cwd);
}

