#ifndef SPM_SPM_H
#define SPM_SPM_H

#include <ctype.h>
#include <errno.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

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

#define PKG_DIR "../pkgs"

#define SHELL_DEFAULT 1L << 0L
#define SHELL_OUTPUT 1L << 1L
#define SHELL_BENCHMARK 1L << 2L
typedef struct {
    struct timespec start_time, stop_time;
    double time_elapsed;
    int returncode;
    char *output;
} Process;

void shell(Process **proc_info, u_int64_t option, const char *fmt, ...);
void shell_free(Process *proc_info);
int tar_extract_file(const char *archive, const char* filename, const char *destination);
int errglob(const char *epath, int eerrno);
int num_chars(const char *sptr, int ch);
int startswith(const char *sptr, const char *pattern);
int endswith(const char *sptr, const char *pattern);
char *normpath(const char *path);
void strchrdel(char *sptr, const char *chars);
long int strchroff(const char *sptr, int ch);
void substrdel(char *sptr, const char *suffix);
char *find_file(const char *root, const char *filename);
char *find_package(const char *filename);
char** split(char *sptr, const char* delim);
void split_free(char **ptr);
char *substring_between(char *sptr, const char *delims);
int has_rpath(const char *filename);
char *get_rpath(const char *filename);

// config.c
#define CONFIG_BUFFER_SIZE 1024

typedef struct {
    char *key;
    char *value;
} Config;

char *lstrip(char *sptr);
char *strip(char *sptr);
int isempty(char *sptr);
int isquoted(char *sptr);
Config **config_read(const char *filename);
void config_free(Config **config);
void config_test(void);

#endif //SPM_SPM_H
