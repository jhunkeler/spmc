/**
 * @file fs.c
 */
#include "spm.h"

/**
 *
 * @param _path
 * @return
 */
FSTreeEx *fstree_ex(const char *_path, char **filter_by, unsigned int filter_mode) {
    FTS *parent = NULL;
    FTSENT *node = NULL;
    FSTreeEx *fsdata = NULL;
    int no_filter = 0;
    char *path = NULL;
    char *abspath = realpath(_path, NULL);

    if (filter_mode & SPM_FSTREE_FLT_RELATIVE) {
        path = strdup(_path);
    } else {
        path = abspath;
    }

    if (path == NULL) {
        spmerrno = errno;
        spmerrno_cause(_path);
        return NULL;
    }
    char *root[2] = { path, NULL };

    if (filter_by == NULL) {
        // Create an array with an empty string. This signifies we want don't want to filter any paths.
        no_filter = 1;
        filter_by = calloc(2, sizeof(char *));
        filter_by[0] = calloc(2, sizeof(char));
        strcpy(filter_by[0], "");
    }

    size_t size = 2;
    size_t records = 0;

    fsdata = (FSTreeEx *)calloc(1, sizeof(FSTree));
    fsdata->root = calloc(PATH_MAX, sizeof(char));
    fsdata->record = calloc(size, sizeof(FSRec *));

    if (filter_mode & SPM_FSTREE_FLT_RELATIVE) {
        // Return an absolute path regardless
        strncpy(fsdata->root, abspath, PATH_MAX - 1);
    } else {
        strncpy(fsdata->root, path, PATH_MAX - 1);
    }

    parent = fts_open(root, FTS_PHYSICAL | FTS_NOCHDIR, &_fstree_compare);

    size_t last_size = size;
    if (parent != NULL) {
        while ((node = fts_read(parent)) != NULL) {
            for (size_t i = 0; filter_by[i] != NULL; i++) {
                // Drop paths containing filter string(s) according to the requested mode
                if (filter_mode & SPM_FSTREE_FLT_CONTAINS && strstr(node->fts_path, filter_by[i]) == NULL) {
                    continue;
                }
                else if (filter_mode & SPM_FSTREE_FLT_ENDSWITH && !endswith(node->fts_path, filter_by[i])) {
                    continue;
                }
                else if (filter_mode & SPM_FSTREE_FLT_STARTSWITH && !startswith(node->fts_path, filter_by[i])) {
                    continue;
                }
                if (strcmp(node->fts_path, "..") == 0 || strcmp(node->fts_path, ".") == 0) {
                    continue;
                }

                FSRec **tmp = (FSRec **) realloc(fsdata->record, sizeof(FSRec *) * size);
                if (tmp == NULL) {
                    spmerrno = errno;
                    spmerrno_cause("Realloc of fsdata failed");
                    return NULL;
                }
                fsdata->record = tmp;
                fsdata->record[last_size] = NULL;
                last_size = size;

                fsdata->record[records] = calloc(1, sizeof(FSRec));
                fsdata->record[records]->name = strdup(node->fts_path);
                if (node->fts_statp) {
                    fsdata->record[records]->st = calloc(1, sizeof(struct stat));
                    memcpy(fsdata->record[records]->st, node->fts_statp, sizeof(struct stat));
                }
                size++;
                records++;
            }
        }
        fts_close(parent);
    }
    fsdata->num_records = records;
    fsdata->_num_alloc = size;
    free(path);
    if (no_filter) {
        free(filter_by[0]);
        free(filter_by);
    }
    return fsdata;
}

/**
 * Free a `FSTree` structure
 * @param fsdata
 */
void fstree_ex_free(FSTreeEx *fsdata) {
    if (fsdata != NULL) {
        if (fsdata->root != NULL) {
            free(fsdata->root);
        }
        if (fsdata->record != NULL) {
            for (int i = 0; i < fsdata->num_records; i++) {
                free(fsdata->record[i]->name);
                free(fsdata->record[i]->st);
            }
            free(fsdata->record);
        }
        free(fsdata);
    }
}

/**
 *
 * @param _path
 * @return
 */
FSTree *fstree(const char *_path, char **filter_by, unsigned int filter_mode) {
    FTS *parent = NULL;
    FTSENT *node = NULL;
    FSTree *fsdata = NULL;
    int no_filter = 0;
    char *path = NULL;
    char *abspath = realpath(_path, NULL);

    if (filter_mode & SPM_FSTREE_FLT_RELATIVE) {
        path = strdup(_path);
    } else {
        path = abspath;
    }

    if (path == NULL) {
        perror(_path);
        fprintf(SYSERROR);
        return NULL;
    }
    char *root[2] = { path, NULL };

    if (filter_by == NULL) {
        // Create an array with an empty string. This signifies we want don't want to filter any paths.
        no_filter = 1;
        filter_by = calloc(2, sizeof(char *));
        filter_by[0] = calloc(2, sizeof(char));
        strcpy(filter_by[0], "");
    }

    size_t dirs_size = 2;
    size_t dirs_records = 0;
    size_t files_size = 2;
    size_t files_records = 0;

    fsdata = (FSTree *)calloc(1, sizeof(FSTree));
    fsdata->root = (char *)calloc(PATH_MAX, sizeof(char));
    fsdata->dirs = (char **)calloc(dirs_size, sizeof(char *));
    fsdata->files = (char **)calloc(files_size, sizeof(char *));

    if (filter_mode & SPM_FSTREE_FLT_RELATIVE) {
        // Return an absolute path regardless
        strncpy(fsdata->root, abspath, PATH_MAX - 1);
    } else {
        strncpy(fsdata->root, path, PATH_MAX - 1);
    }

    parent = fts_open(root, FTS_PHYSICAL | FTS_NOCHDIR, &_fstree_compare);

    if (parent != NULL) {
        while ((node = fts_read(parent)) != NULL) {
            for (size_t i = 0; filter_by[i] != NULL; i++) {
                // Drop paths containing filter string(s) according to the requested mode
                if (filter_mode & SPM_FSTREE_FLT_CONTAINS && strstr(node->fts_path, filter_by[i]) == NULL) {
                    continue;
                }
                else if (filter_mode & SPM_FSTREE_FLT_ENDSWITH && !endswith(node->fts_path, filter_by[i])) {
                    continue;
                }
                else if (filter_mode & SPM_FSTREE_FLT_STARTSWITH && !startswith(node->fts_path, filter_by[i])) {
                    continue;
                }
                switch (node->fts_info) {
                    case FTS_D:
                        if (strcmp(node->fts_path, "..") == 0 || strcmp(node->fts_path, ".") == 0) {
                            continue;
                        }
                        fsdata->dirs = (char **) realloc(fsdata->dirs, sizeof(char *) * dirs_size);
                        fsdata->dirs[dirs_size - 1] = NULL;
                        fsdata->dirs[dirs_records] = (char *) calloc(strlen(node->fts_path) + 1, sizeof(char));
                        strncpy(fsdata->dirs[dirs_records], node->fts_path, strlen(node->fts_path));
                        dirs_size++;
                        dirs_records++;
                        break;
                    case FTS_F:
                    case FTS_SL:
                        fsdata->files = (char **) realloc(fsdata->files, sizeof(char *) * files_size);
                        fsdata->files[files_size - 1] = NULL;
                        fsdata->files[files_records] = (char *) calloc(strlen(node->fts_path) + 1, sizeof(char));
                        strncpy(fsdata->files[files_records], node->fts_path, strlen(node->fts_path));
                        files_size++;
                        files_records++;
                        break;
                    default:
                        break;
                }
            }
        }
        fts_close(parent);
    }
    fsdata->dirs_length = dirs_records;
    fsdata->files_length = files_records;
    free(path);
    if (no_filter) {
        free(filter_by[0]);
        free(filter_by);
    }
    return fsdata;
}

/**
 *
 * @param one
 * @param two
 * @return
 */
int _fstree_compare(const FTSENT **one, const FTSENT **two) {
    return (strcmp((*one)->fts_name, (*two)->fts_name));
}

/**
 * Produce a zero-depth directory listing
 *
 * @param path absolute or relative path string (directory)
 * @return error=NULL, success=`FSList` structure
 */
FSList *fslist(const char *path) {
    FSList *tree = NULL;
    DIR *dir = NULL;
    struct dirent *d_ent = NULL;

    if (path == NULL) {
        return NULL;
    }

    tree = calloc(1, sizeof(FSList));
    if (tree == NULL) {
        perror(path);
        return NULL;
    }

    tree->root = strdup(path);
    tree->records = 0;
    tree->_num_alloc = 1;

    tree->record = calloc(tree->_num_alloc, sizeof(struct dirent *));
    if (tree->record == NULL) {
        perror("tree->record");
        goto failed;
    }

    if ((dir = opendir(tree->root)) == NULL) {
        perror(tree->root);
        goto failed;
    }

    errno = 0;  // clear errno so perror prints the correct error
    while ((d_ent = readdir(dir)) != NULL) {
        struct dirent **tmp = NULL;
        if (strcmp(d_ent->d_name, ".") == 0 || strcmp(d_ent->d_name, "..") == 0) {
            continue;
        }

        tree->record[tree->records] = calloc(1, sizeof(struct dirent));
        memcpy(tree->record[tree->records], d_ent, sizeof(struct dirent));

        if (tree->record[tree->records] == NULL) {
            perror("unable to allocate record for tree->files");
            goto failed;
        }

        tree->records++;
        tree->_num_alloc++;

        tmp = realloc(tree->record, tree->_num_alloc * sizeof(struct dirent *));
        if (tmp == NULL) {
            perror("unable to reallocate tree->record");
            goto failed;
        }
        tree->record = tmp;
        tree->record[tree->records] = NULL;
    }

    if (errno) {
        perror(path);
failed: // label
        fslist_free(tree);
        return NULL;
    }

    closedir(dir);
    return tree;
}

/**
 *
 * @param _path
 * @return
 */
int rmdirs(const char *_path) {
    if (access(_path, F_OK) != 0) {
        return -1;
    }

    FSTree *data = fstree(_path, NULL, SPM_FSTREE_FLT_NONE);
    if (data->files) {
        for (size_t i = 0; data->files[i] != NULL; i++) {
            remove(data->files[i]);
        }
    }
    if (data->dirs) {
        for (size_t i = data->dirs_length - 1; i != 0; i--) {
            remove(data->dirs[i]);
        }
    }
    remove(data->root);

    fstree_free(data);
    return 0;
}

/**
 * Free a `FSTree` structure
 * @param fsdata
 */
void fstree_free(FSTree *fsdata) {
    if (fsdata != NULL) {
        if (fsdata->root != NULL) {
            free(fsdata->root);
        }
        if (fsdata->files != NULL) {
            for (int i = 0; fsdata->files[i] != NULL; i++) {
                free(fsdata->files[i]);
            }
            free(fsdata->files);
        }
        if (fsdata->dirs != NULL) {
            for (int i = 0; fsdata->dirs[i] != NULL; i++) {
                free(fsdata->dirs[i]);
            }
            free(fsdata->dirs);
        }
        free(fsdata);
    }
}

void fslist_free(FSList *fsdata) {
    if (fsdata == NULL) {
        return;
    }

    if (fsdata->root != NULL) {
        free(fsdata->root);
    }
    if (fsdata->record != NULL) {
        for (size_t i = 0; fsdata->record[i] != NULL; i++) {
            free(fsdata->record[i]);
        }
    }
    free(fsdata->record);
    free(fsdata);
}

/**
 * Expand "~" to the user's home directory
 *
 * Example:
 * ~~~{.c}
 * char *home = expandpath("~");             // == /home/username
 * char *config = expandpath("~/.config");   // == /home/username/.config
 * char *nope = expandpath("/tmp/test");     // == /tmp/test
 * char *nada = expandpath("/~/broken");     // == /~/broken
 *
 * free(home);
 * free(config);
 * free(nope);
 * free(nada);
 * ~~~
 *
 * @param _path (Must start with a `~`)
 * @return success=expanded path or original path, failure=NULL
 */
char *expandpath(const char *_path) {
    if (_path == NULL) {
        return NULL;
    }
    const char *homes[] = {
            "HOME",
            "USERPROFILE",
    };
    char home[PATH_MAX];
    char tmp[PATH_MAX];
    char *ptmp = tmp;
    char result[PATH_MAX];
    char *sep = NULL;

    memset(home, '\0', sizeof(home));
    memset(ptmp, '\0', sizeof(tmp));
    memset(result, '\0', sizeof(result));

    strncpy(ptmp, _path, PATH_MAX - 1);

    // Check whether there's a reason to continue processing the string
    if (*ptmp != '~') {
        return strdup(ptmp);
    }

    // Remove tilde from the string and shift its contents to the left
    strchrdel(ptmp, "~");

    // Figure out where the user's home directory resides
    for (size_t i = 0; i < sizeof(homes); i++) {
        char *tmphome;
        if ((tmphome = getenv(homes[i])) != NULL) {
            strncpy(home, tmphome, PATH_MAX - 1);
            break;
        }
    }

    // A broken runtime environment means we can't do anything else here
    if (isempty(home)) {
        return NULL;
    }

    // Scan the path for a directory separator
    if ((sep = strpbrk(ptmp, "/\\")) != NULL) {
        // Jump past it
        ptmp = sep + 1;
    }

    // Construct the new path
    strncat(result, home, PATH_MAX - 1);
    if (sep) {
        strncat(result, DIRSEPS, PATH_MAX - 1);
        strncat(result, ptmp, PATH_MAX - 1);
    }

    return strdup(result);
}

/**
 * Converts Win32 path to Unix path, and vice versa
 *  - On UNIX, Win32 paths will be converted UNIX
 *  - On Win32, UNIX paths will be converted to Win32
 *
 * This function is platform dependent. The string is modified in-place.
 *
 * @param path a system path
 * @return string
 */
char *normpath(const char *path) {
    char *result = strdup(path);
    char *tmp = result;

    while (*tmp) {
        if (*tmp == NOT_DIRSEP) {
            *tmp = DIRSEP;
            tmp++;
            continue;
        }
        tmp++;
    }
    return result;
}


/**
 * Strip file name from directory
 * Note: Caller is responsible for freeing memory
 *
 * @param _path
 * @return success=path to directory, failure=NULL
 */
char *dirname(const char *_path) {
    if (_path == NULL) {
        return NULL;
    }
    char *path = strdup(_path);
    char *last = strrchr(path, DIRSEP);
    if (!last) {
        return NULL;
    }
    // Step backward, stopping on the first non-separator
    // This ensures strings like "/usr//////" are converted to "/usr", but...
    // it will do nothing to fix up a path like "/usr//////bin/bash
    char *lookback = last;
    while (*(lookback - 1) == DIRSEP) {
        lookback--;
    }

    *lookback = '\0';
    return path;
}

/**
 * Strip directory from file name
 * Note: Caller is responsible for freeing memory
 *
 * @param _path
 * @return success=file name, failure=NULL
 */
char *basename(char *path) {
    char *result = NULL;
    char *last = NULL;

    if ((last = strrchr(path, DIRSEP)) == NULL) {
        return result;
    }
    // Perform a lookahead ensuring the string is valid beyond the last separator
    if (last++ != NULL) {
        result = last;
    }

    return result;
}

/**
 * Basic rsync wrapper for copying files
 * @param _args arguments to pass to rsync (set to `NULL` for default options)
 * @param _source source file or directory
 * @param _destination destination file or directory
 * @return success=0, failure=-1
 */
int rsync(const char *_args, const char *_source, const char *_destination) {
    int returncode;
    Process *proc = NULL;
    char *args = NULL;
    if (_args) {
        args = strdup(_args);
    }
    char *source = strdup(_source);
    char *destination = strdup(_destination);
    char cmd[PATH_MAX];
    char *args_combined = (char *)calloc(PATH_MAX, sizeof(char));

    memset(cmd, '\0', sizeof(cmd));
    strcpy(args_combined, "--archive --hard-links ");
    if (args) {
        strcat(args_combined, _args);
    }

    strchrdel(args_combined, SHELL_INVALID);
    strchrdel(source, SHELL_INVALID);
    strchrdel(destination, SHELL_INVALID);

    snprintf(cmd, PATH_MAX, "rsync %s \"%s\" \"%s\" 2>&1", args_combined, source, destination);
    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        if (args) {
            free(args);
        }
        free(source);
        free(destination);
        return -1;
    }

    returncode = proc->returncode;
    if (returncode != 0 && proc->output) {
        fprintf(stderr, "%s\n", proc->output);
    }
    shell_free(proc);

    if (args) {
        free(args);
    }
    free(args_combined);
    free(source);
    free(destination);
    return returncode;
}

/**
 * Return the size of a file
 * @param filename
 * @return
 */
long int get_file_size(const char *filename) {
    long int result = 0;
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return -1;
    }
    if (fseek(fp, 0, SEEK_END) < 0) {
        return -1;
    }
    result = ftell(fp);
    fclose(fp);
    return result;
}

/**
 * Attempt to create a directory (or directories)
 * @param _path A path to create
 * @param mode UNIX permissions (octal)
 * @return success=0, failure=-1 (+ errno will be set)
 */
int mkdirs(const char *_path, mode_t mode) {
    int result = 0;
    char *path = normpath(_path);
    char tmp[PATH_MAX];
    tmp[0] = '\0';

    char sep[2];
    sprintf(sep, "%c", DIRSEP);
    char **parts = split(path, sep);
    for (int i = 0; parts[i] != NULL; i++) {
        strcat(tmp, parts[i]);
        strcat(tmp, sep);
        if (access(tmp, F_OK) != 0) {
            result = mkdir(tmp, mode);
        }
    }
    split_free(parts);
    free(path);
    return result;
}

/**
 * Short wrapper for `access`. Check if file exists.
 *
 * Example:
 * ~~~{.c}
 * if (exists("example.txt") != 0) {
 *     // handle error
 * }
 * ~~~
 * @param filename
 * @return
 */
int exists(const char *filename) {
    return access(filename, F_OK);
}

/**
 * Convert size in bytes to the closest human-readable unit
 *
 * NOTE: Caller is responsible for freeing memory
 *
 * Example:
 * ~~~{.c}
 * char *output;
 * output = human_readable_size(1);       // "1B"
 * free(output);
 * output = human_readable_size(1024)     // "1.0K"
 * free(output);
 * output = human_readable_size(1024000)  // "1.0M"
 * free(output);
 * // and so on
 * ~~~
 *
 * @param n size to convert
 * @return string
 */
char *human_readable_size(uint64_t n) {
    size_t i;
    double result = (double)n;
    char *unit[] = {"B", "K", "M", "G", "T", "P", "E"};
    char r[255];
    memset(r, '\0', sizeof(r));

    for (i = 0; i < sizeof(unit); i++) {
        if (fabs(result) < 1024) {
            break;
        }
        result /= 1024.0;
    }

    if (unit[i][0] == 'B') {
        sprintf(r, "%0.0lf%s", result, unit[i]);
    }
    else {
        sprintf(r, "%0.2lf%s", result, unit[i]);
    }

    return strdup(r);
}

/**
 * Create a named temporary directory
 * @param base path where temporary directory will be created
 * @param name name of temporary directory
 * @param extended_path a subdirectory to create beneath `base/name`, may be NULL
 * @return success=path, failure=NULL
 */
char *spm_mkdtemp(const char *base, const char *name, const char *extended_path) {
    const char *template_unique = "XXXXXX";
    char *tmpdir = NULL;
    char template[PATH_MAX];

    if (base == NULL || name == NULL) {  // extended_path is optional
        return NULL;
    }

    sprintf(template, "%s%s%s_%s", base, DIRSEPS, name, template_unique);
    tmpdir = mkdtemp(template);

    if (tmpdir == NULL) {
        return NULL;
    }

    if (extended_path != NULL) {
        char extended[PATH_MAX] = {0,};
        strncpy(extended, tmpdir, PATH_MAX - 1);
        strcat(extended, DIRSEPS);
        strcat(extended, extended_path);
        mkdirs(extended, 0755);
    }
    return strdup(tmpdir);
}

 /**
  * Create an empty file or update a file's modified timestamp
  * @param path path to file
  * @return error=-1, success=0
  */
 int touch(const char *path) {
     FILE *fp = NULL;
     struct stat st;
     int path_stat = 0;

     if (path == NULL) {
         return -1;
     }

     path_stat = stat(path, &st);
     if (path_stat < 0) {
         if ((fp = fopen(path, "w+")) == NULL) {
             return -1;
         }
         fclose(fp);
     } else {
         if ((S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) > 0) {
             if (utime(path, NULL) < 0) {
                 // failed to set file time
                 return -1;
             }
             // updated file time
         } else {
             return -1;
         }
     }

     return 0;
 }

char **file_readlines(const char *filename, size_t start, size_t limit, ReaderFn *readerFn) {
    FILE *fp = NULL;
    char **result = NULL;
    char *buffer = NULL;
    size_t lines = 0;
    int use_stdin = 0;

    if (strcmp(filename, "-") == 0) {
        use_stdin = 1;
    }

    if (use_stdin) {
        fp = stdin;
    } else {
        fp = fopen(filename, "r");
    }

    if (fp == NULL) {
        perror(filename);
        fprintf(SYSERROR);
        return NULL;
    }

    // Allocate buffer
    if ((buffer = calloc(BUFSIZ, sizeof(char))) == NULL) {
        perror("line buffer");
        fprintf(SYSERROR);
        if (!use_stdin) {
            fclose(fp);
        }
        return NULL;
    }

    // count number the of lines in the file
    while ((fgets(buffer, BUFSIZ - 1, fp)) != NULL) {
        lines++;
    }

    if (!lines) {
        free(buffer);
        if (!use_stdin) {
            fclose(fp);
        }
        return NULL;
    }

    rewind(fp);

    // Handle invalid start offset
    if (start > lines) {
        start = 0;
    }

    // Adjust line count when start offset is non-zero
    if (start != 0 && start < lines) {
        lines -= start;
    }


    // Handle minimum and maximum limits
    if (limit == 0 || limit > lines) {
        limit = lines;
    }

    // Populate results array
    result = calloc(limit + 1, sizeof(char *));
    for (size_t i = start; i < limit; i++) {
        if (i < start) {
            continue;
        }

        if (fgets(buffer, BUFSIZ - 1, fp) == NULL) {
            break;
        }

        if (readerFn != NULL) {
            int status = readerFn(i - start, &buffer);
            // A status greater than zero indicates we should ignore this line entirely and "continue"
            // A status less than zero indicates we should "break"
            // A zero status proceeds normally
            if (status > 0) {
                i--;
                continue;
            } else if (status < 0) {
                break;
            }
        }
        result[i] = strdup(buffer);
        memset(buffer, '\0', BUFSIZ);
    }

    free(buffer);
    if (!use_stdin) {
        fclose(fp);
    }
    return result;
}