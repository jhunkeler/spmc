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
