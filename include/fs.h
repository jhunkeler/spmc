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

int _fstree_compare(const FTSENT **a, const FTSENT **b);
void fstree_free(FSTree *fsdata);
FSTree *fstree(const char *_path, char **filter_by, unsigned int filter_mode);
int exists(const char *filename);
int rmdirs(const char *_path);
long int get_file_size(const char *filename);
int mkdirs(const char *_path, mode_t mode);
char *dirname(const char *_path);
char *basename(char *path);
int rsync(const char *_args, const char *_source, const char *_destination);
char *human_readable_size(uint64_t n);
char *expandpath(const char *_path);
char *spm_mkdtemp(const char *name);


#endif //SPM_FSTREE_H
