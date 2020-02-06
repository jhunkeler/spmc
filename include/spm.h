/**
 * @file spm.h
 */
#ifndef SPM_SPM_H
#define SPM_SPM_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fts.h>
#include <glob.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <wordexp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#if !defined(_WIN32)
#include <sys/utsname.h>
#endif

#include "strlist.h"
#include "config.h"

// spm.c
#define SYSERROR stderr, "%s:%s:%d: %s\n", basename(__FILE__), __FUNCTION__, __LINE__, strerror(errno)
#define DIRSEP_WIN32 '\\'
#define DIRSEPS_WIN32 "\\"
#define PATHSEP_WIN32 ';'
#define PATHSEPS_WIN32 ";"
#define DIRSEP_UNIX '/'
#define DIRSEPS_UNIX "/"
#define PATHSEP_UNIX ';'
#define PATHSEPS_UNIX ";"
#if defined(_WIN32)
#define DIRSEP  DIRSEP_WIN32
#define DIRSEPS  DIRSEPS_WIN32
#define NOT_DIRSEP DIRSEP_UNIX
#define NOT_DIRSEPS DIRSEPS_UNIX

#define PATHSEP PATHSEP_WIN32
#define PATHSEPS PATHSEPS_WIN32
#define NOT_PATHSEP PATHSEP_UNIX
#define NOT_PATHSEPS PATHSEPS_UNIX
#else
#define DIRSEP DIRSEP_UNIX
#define DIRSEPS DIRSEPS_UNIX
#define NOT_DIRSEP DIRSEP_WIN32
#define NOT_DIRSEPS DIRSEPS_WIN32

#define PATHSEP PATHSEP_UNIX
#define PATHSEPS PATHSEPS_UNIX
#define NOT_PATHSEP PATHSEP_WIN32
#define NOT_PATHSEPS PATHSEPS_WIN32
#endif

#define SPM_META_DEPENDS ".SPM_DEPENDS"
#define SPM_META_PREFIX_BIN ".SPM_PREFIX_BIN"
#define SPM_META_PREFIX_TEXT ".SPM_PREFIX_TEXT"
#define SPM_META_MANIFEST ".SPM_MANIFEST" // TODO: Implement

#define SPM_MANIFEST_SEPARATOR '|'
#define SPM_MANIFEST_SEPARATOR_MAX 7
#define SPM_MANIFEST_NODATA "*"
#define SPM_MANIFEST_HEADER "# SPM PACKAGE MANIFEST"
#define SPM_MANIFEST_FILENAME "manifest.dat"

#define SPM_FSTREE_FLT_NONE 1 << 0
#define SPM_FSTREE_FLT_CONTAINS 1 << 1
#define SPM_FSTREE_FLT_ENDSWITH 1 << 2
#define SPM_FSTREE_FLT_STARTSWITH 1 << 3

#define PREFIX_WRITE_BIN 0
#define PREFIX_WRITE_TEXT 1

#define SPM_PACKAGE_EXTENSION ".tar.gz"
#define PKG_DIR SPM_GLOBAL.package_dir
#define TMP_DIR SPM_GLOBAL.tmp_dir

#define SHELL_DEFAULT 1 << 0
#define SHELL_OUTPUT 1 << 1
#define SHELL_BENCHMARK 1 << 2

#define PACKAGE_MEMBER_SIZE 0xff
#define PACKAGE_MEMBER_ORIGIN_SIZE PATH_MAX
#define PACKAGE_MEMBER_SEPARATOR '-'
#define PACKAGE_MEMBER_SEPARATOR_PLACEHOLD '*'

#define VERSION_OPERATORS " ~!=<>"
#define VERSION_NOOP 1 << 0
#define VERSION_EQ 1 << 1
#define VERSION_NE 1 << 2
#define VERSION_GT 1 << 3
#define VERSION_LT 1 << 4
#define VERSION_COMPAT 1 << 5

#define SPM_MIRROR_MAX 0xff
#define SPM_MIRROR_FILENAME "mirrorlist"

#define SHA256_DIGEST_STRING_LENGTH (SHA256_DIGEST_LENGTH * 2) + 1

typedef struct {
    char **requirements;
    size_t requirements_records;
    size_t size;
    char archive[PACKAGE_MEMBER_SIZE];
    char name[PACKAGE_MEMBER_SIZE];
    char version[PACKAGE_MEMBER_SIZE];
    char revision[PACKAGE_MEMBER_SIZE];
    char checksum_sha256[SHA256_DIGEST_STRING_LENGTH];
    char origin[PACKAGE_MEMBER_ORIGIN_SIZE];
} ManifestPackage;

typedef struct {
    size_t records;
    ManifestPackage **packages;
    char origin[PACKAGE_MEMBER_ORIGIN_SIZE];
} Manifest;

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
    char *binpath;
    char *includepath;
    char *libpath;
    char *datapath;
    char *manpath;
} SPM_Hierarchy;

typedef struct {
    char *package_dir;
    char *tmp_dir;
    char *package_manifest;
    char *mirror_config;
    char **mirror_list;
    char *repo_target;
    char *user_config_basedir;
    char *user_config_file;
    int verbose;
    ConfigItem **config;
    struct utsname sysinfo;
    SPM_Hierarchy fs;
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

typedef struct {
    char *origin;
    char *type;
    char *charset;
} Mime;

typedef StrList RuntimeEnv;
//typedef StrList Dependencies;

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
int prefixes_write(const char *output_file, int mode, char **prefix, const char *tree);

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
char *join(char **arr, const char *separator);
char *substring_between(char *sptr, const char *delims);
void strsort(char **arr);
int find_in_file(const char *filename, const char *pattern);
int isrelational(char ch);
void print_banner(const char *s, int len);
char *strstr_array(char **arr, const char *str);
char **strdeldup(char **arr);
char *lstrip(char *sptr);
char *strip(char *sptr);
int isempty(char *sptr);
int isquoted(char *sptr);
char *normalize_space(char *s);

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
int rpath_autoset(const char *filename);
int rpath_set(const char *filename, const char *rpath);

// fs.c
int _fstree_compare(const FTSENT **a, const FTSENT **b);
void fstree_free(FSTree *fsdata);
FSTree *fstree(const char *_path, char **filter_by, unsigned int filter_mode);
int rmdirs(const char *_path);
long int get_file_size(const char *filename);
int mkdirs(const char *_path, mode_t mode);
char *dirname(const char *_path);
char *basename(char *path);
int rsync(const char *_args, const char *_source, const char *_destination);
char *human_readable_size(uint64_t n);

// config_global.c
char *get_user_conf_dir(void);
char *get_user_config_file(void);
char *get_user_tmp_dir(void);
char *get_user_package_dir(void);
char *get_package_manifest(void);

void init_config_global(void);
void free_global_config(void);
void show_global_config(void);
void check_runtime_environment(void);

// install.c
int metadata_remove(const char *_path);
int install(const char *destroot, const char *_package);

// config.c
#define CONFIG_BUFFER_SIZE 1024

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

// manifest.c
int fetch(const char *url, const char *dest);
void manifest_package_separator_swap(char **name);
void manifest_package_separator_restore(char **name);
Manifest *manifest_from(const char *package_dir);
Manifest *manifest_read(char *file_or_url);
int manifest_write(Manifest *info, const char *dest);
void manifest_free(Manifest *info);
void manifest_package_free(ManifestPackage *info);
ManifestPackage *manifest_search(Manifest *info, const char *package);
ManifestPackage *find_by_strspec(Manifest *manifest, const char *_strspec);
ManifestPackage *manifest_package_copy(ManifestPackage *manifest);


// checksum.c
char *md5sum(const char *filename);
char *sha256sum(const char *filename);

// version_spec.c
char *version_suffix_get_alpha(char *str);
char *version_suffix_get_modifier(char *str);
int64_t version_suffix_modifier_calc(char *str);
int version_suffix_alpha_calc(char *str);
int64_t version_from(const char *version_str);
int version_spec_from(const char *op);
ManifestPackage **find_by_spec(Manifest *manifest, const char *name, const char *op, const char *version_str);

// build.c
Process *file_command(const char *_filename);
Mime *file_mimetype(const char *filename);
void mime_free(Mime *m);
int build(int bargc, char **bargv);
int file_is_binary(const char *filename);
int file_is_text(const char *filename);
int file_is_binexec(const char *filename);

// internal_cmd.c
int internal_cmd(int argc, char **argv);

// environment.c
ssize_t runtime_contains(RuntimeEnv *env, const char *key);
RuntimeEnv *runtime_copy(char **env);
char *runtime_get(RuntimeEnv *env, const char *key);
void runtime_set(RuntimeEnv *env, const char *_key, const char *_value);
char *runtime_expand_var(RuntimeEnv *env, const char *input);
void runtime_export(RuntimeEnv *env, char **keys);
void runtime_apply(RuntimeEnv *env);
void runtime_free(RuntimeEnv *env);

// mirrors.c
char **file_readlines(const char *filename);
char **mirror_list(const char *filename);
void mirror_list_free(char **m);
void mirror_clone(Manifest *info, char *dest);

#endif //SPM_SPM_H
