#include "spm.h"

FSTree *fstree(const char *_path) {
    FTS *parent = NULL;
    FTSENT *node = NULL;
    FSTree *fsdata = NULL;
    char *path = realpath(_path, NULL);
    char *root[2] = { path, NULL };

    size_t dirs_size = 2;
    size_t dirs_records = 0;
    size_t files_size = 2;
    size_t files_records = 0;

    fsdata = (FSTree *)calloc(1, sizeof(FSTree));
    fsdata->root= (char *)calloc(strlen(path) + 1, sizeof(char));
    fsdata->dirs = (char **)calloc(dirs_size, sizeof(char *));
    fsdata->files= (char **)calloc(files_size, sizeof(char *));

    strncpy(fsdata->root, path, strlen(path));
    parent = fts_open(root, FTS_PHYSICAL | FTS_NOCHDIR, &_fstree_compare);

    if (parent != NULL) {
        while ((node = fts_read(parent)) != NULL) {
            switch (node->fts_info) {
                case FTS_D:
                    if (strcmp(node->fts_path, "..") == 0 || strcmp(node->fts_path, ".") == 0) {
                        continue;
                    }
                    fsdata->dirs = (char **)realloc(fsdata->dirs, sizeof(char*) * dirs_size);
                    fsdata->dirs[dirs_size - 1] = NULL;
                    fsdata->dirs[dirs_records] = (char *)calloc(strlen(node->fts_path) + 1, sizeof(char));
                    strncpy(fsdata->dirs[dirs_records], node->fts_path, strlen(node->fts_path));
                    dirs_size++;
                    dirs_records++;
                    break;
                case FTS_F:
                case FTS_SL:
                    fsdata->files = (char **)realloc(fsdata->files, sizeof(char*) * files_size);
                    fsdata->files[files_size - 1] = NULL;
                    fsdata->files[files_records] = (char *)calloc(strlen(node->fts_path) + 1, sizeof(char));
                    strncpy(fsdata->files[files_records], node->fts_path, strlen(node->fts_path));
                    files_size++;
                    files_records++;
                    break;
                default:
                    break;
            }
        }
        fts_close(parent);
    }
    fsdata->dirs_length = dirs_records;
    fsdata->files_length = files_records;
    free(path);
    return fsdata;
}

int _fstree_compare(const FTSENT **one, const FTSENT **two) {
    return (strcmp((*one)->fts_name, (*two)->fts_name));
}

int rmdirs(const char *_path) {
    if (access(_path, F_OK) != 0) {
        return -1;
    }

    FSTree *data = fstree(_path);
    if (data->files) {
        for (int i = 0; data->files[i] != NULL; i++) {
            remove(data->files[i]);
        }
    }
    if (data->dirs) {
        for (int i = data->dirs_length - 1; i != 0; i--) {
            remove(data->dirs[i]);
        }
    }
    remove(data->root);

    fstree_free(data);
    return 0;
}

void fstree_free(FSTree *fsdata) {
    if (fsdata != NULL) {
        if (fsdata->root != NULL) {
            free(fsdata->root);
        }
        if (fsdata->files != NULL) {
            for (int i = 0; fsdata->files[i] != NULL; i++) {
                free(fsdata->files[i]);
            }
        }
        if (fsdata->dirs != NULL) {
            for (int i = 0; fsdata->dirs[i] != NULL; i++) {
                free(fsdata->dirs[i]);
            }
        }
        free(fsdata);
    }
}

/**
 * Converts Win32 path to Unix path, and vice versa
 *  - On UNIX, Win32 paths will be converted UNIX
 *  - On Win32, UNIX paths will be converted to Win32
 *
 * This function is platform dependent.
 *
 * @param path a system path
 * @return string (caller is responsible for `free`ing memory)
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
    char *last = strrchr(path, DIRSEP);
    if (!last) {
        return NULL;
    }

    // Perform a lookahead ensuring the string is valid beyond the last separator
    if ((last + 1) != NULL) {
        result = last + 1;
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
    char args_combined[PATH_MAX];

    memset(cmd, '\0', sizeof(cmd));
    memset(args_combined, '\0', sizeof(args_combined));
    strcpy(args_combined, "--archive --hard-links ");
    if (args) {
        strcat(args_combined, _args);
    }

    sprintf(cmd, "rsync %s \"%s\" \"%s\"", args_combined, source, destination);
    // sanitize command
    strchrdel(cmd, "&;|");
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
        fprintf(stderr, proc->output);
    }
    shell_free(proc);

    if (args) {
        free(args);
    }
    free(source);
    free(destination);
    return returncode;
}

long int get_file_size(const char *filename) {
    long int result = 0;
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
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
    return result;
}
