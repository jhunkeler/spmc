#ifndef SPM_SPM_H
#define SPM_SPM_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fts.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <wordexp.h>
#if !defined(_WIN32)
#include <sys/utsname.h>
#endif

#include "config.h"

// spm.c
#define SYSERROR stderr, "%s:%s:%d: %s\n", __FILE__, __FUNCTION__, __LINE__, strerror(errno)
#define DIRSEP_WIN32 '\\'
#define DIRSEP_UNIX '/'
#if defined(_WIN32)
#define DIRSEP  DIRSEP_WIN32
#define NOT_DIRSEP DIRSEP_UNIX
#else
#define DIRSEP DIRSEP_UNIX
#define NOT_DIRSEP DIRSEP_WIN32
#endif

#define PKG_DIR SPM_GLOBAL.package_dir
#define TMP_DIR SPM_GLOBAL.tmp_dir

#define SHELL_DEFAULT 1 << 0
#define SHELL_OUTPUT 1 << 1
#define SHELL_BENCHMARK 1 << 2

typedef struct {
    char *root;
    char **dirs;
    size_t dirs_length;
    char **files;
    size_t files_length;
} FSTree;

typedef struct {
    size_t __size;      // Count of allocated records
    size_t records;     // Count of usable records
    char **list;        // Array of dependencies
} Dependencies;

typedef struct {
    char *key;
    char *value;
    size_t key_length;
    size_t value_length;
} ConfigItem;

typedef struct {
    char *package_dir;
    char *tmp_dir;
    char *user_config_basedir;
    char *user_config_file;
    ConfigItem **config;
    struct utsname sysinfo;
} spm_vars;

typedef struct {
    struct timespec start_time, stop_time;
    double time_elapsed;
    int returncode;
    char *output;
} Process;

typedef struct {
    char *prefix;
    char *path;
} RelocationEntry;

// GLOBALS
spm_vars SPM_GLOBAL;

// shell.c
void shell(Process **proc_info, u_int64_t option, const char *fmt, ...);
void shell_free(Process *proc_info);

// archive.c
int tar_extract_archive(const char *_archive, const char *_destination);
int tar_extract_file(const char *archive, const char* filename, const char *destination);

// relocation.c
int relocate(const char *filename, const char *_oldstr, const char *_newstr);
int replace_text(char *data, const char *_spattern, const char *_sreplacement);
int file_replace_text(char *filename, const char *spattern, const char *sreplacement);
RelocationEntry **prefixes_read(const char *filename);
void prefixes_free(RelocationEntry **entry);

// strings.c
int num_chars(const char *sptr, int ch);
int startswith(const char *sptr, const char *pattern);
int endswith(const char *sptr, const char *pattern);
char *normpath(const char *path);
void strchrdel(char *sptr, const char *chars);
long int strchroff(const char *sptr, int ch);
void strdelsuffix(char *sptr, const char *suffix);
char** split(char *sptr, const char* delim);
void split_free(char **ptr);
char *substring_between(char *sptr, const char *delims);
static int _strsort_compare(const void *a, const void *b);
void strsort(char **arr);
int find_in_file(const char *filename, const char *pattern);

// find.c
char *find_executable(const char *program);
char *find_file(const char *root, const char *filename);
char *find_package(const char *filename);
int errglob(const char *epath, int eerrno);

// rpath.c
Process *patchelf(const char *_filename, const char *_args);
char *rpath_autodetect(const char *filename);
int has_rpath(const char *_filename);
char *rpath_get(const char *_filename);
char *rpath_generate(const char *_filename);
int rpath_set(const char *filename, char *_rpath);

// fs.c
long int get_file_size(const char *filename);
int mkdirs(const char *_path, mode_t mode);
char *dirname(const char *_path);
char *basename(char *path);
int rsync(const char *_args, const char *_source, const char *_destination);

// config_global.c
char *get_user_conf_dir(void);
char *get_user_config_file(void);
char *get_user_tmp_dir(void);
char *get_user_package_dir(void);

void init_config_global(void);
void free_global_config(void);
void show_global_config(void);
void check_runtime_environment(void);

// install.c
int install(const char *destroot, const char *_package);

// config.c
#define CONFIG_BUFFER_SIZE 1024

char *lstrip(char *sptr);
char *strip(char *sptr);
int isempty(char *sptr);
int isquoted(char *sptr);
ConfigItem **config_read(const char *filename);
ConfigItem *config_get(ConfigItem **item, const char *key);
void config_free(ConfigItem **item);
void config_test(void);

// deps.c
int exists(const char *filename);
int dep_seen(Dependencies **deps, const char *name);
int dep_init(Dependencies **deps);
void dep_free(Dependencies **deps);
int dep_append(Dependencies **deps, char *name);
int dep_solve(Dependencies **deps, const char *filename);
int dep_all(Dependencies **deps, const char *_package);
void dep_show(Dependencies **deps);

// fstree.c
int _fstree_compare(const FTSENT **a, const FTSENT **b);
void fstree_free(FSTree *fsdata);
FSTree *fstree(const char *_path);
int rmdirs(const char *_path);

#endif //SPM_SPM_H
