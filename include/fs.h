/**
 * @file fs.h
 */
#ifndef SPM_FSTREE_H
#define SPM_FSTREE_H

#define SPM_FSTREE_FLT_NONE 1 << 0
#define SPM_FSTREE_FLT_CONTAINS 1 << 1
#define SPM_FSTREE_FLT_ENDSWITH 1 << 2
#define SPM_FSTREE_FLT_STARTSWITH 1 << 3
#define SPM_FSTREE_FLT_RELATIVE 1 << 4

typedef struct {
    char *root;
    char **dirs;
    size_t dirs_length;
    char **files;
    size_t files_length;
} FSTree;

typedef struct {
    char *root;
    struct dirent **record;
    size_t records;
    size_t _num_alloc;
} FSList;

int _fstree_compare(const FTSENT **a, const FTSENT **b);
FSTree *fstree(const char *_path, char **filter_by, unsigned int filter_mode);
void fstree_free(FSTree *fsdata);
FSList *fslist(const char *path);
void fslist_free(FSList *fsdata);
int exists(const char *filename);
int rmdirs(const char *_path);
long int get_file_size(const char *filename);
int mkdirs(const char *_path, mode_t mode);
char *dirname(const char *_path);
char *basename(char *path);
int rsync(const char *_args, const char *_source, const char *_destination);
char *human_readable_size(uint64_t n);
char *expandpath(const char *_path);
char *spm_mkdtemp(const char *name, const char *extended_path);
int touch(const char *path);


#endif //SPM_FSTREE_H
