/**
 * @file spm.h
 */
#ifndef SPM_SPM_H
#define SPM_SPM_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#if !defined(_WIN32)
#include <fts.h>
#include <glob.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <utime.h>
#endif

#include "package.h"
#include "str.h"
#include "strlist.h"
#include "shlib.h"
#include "config.h"
#include "internal_cmd.h"
#include "environment.h"
#include "metadata.h"
#include "manifest.h"
#include "fs.h"
#include "version_spec.h"
#include "checksum.h"
#include "resolve.h"
#include "shell.h"
#include "relocation.h"
#include "conf.h"
#include "archive.h"
#include "find.h"
#include "rpath.h"
#include "mime.h"
#include "mirrors.h"
#include "user_input.h"
#include "install.h"
#include "purge.h"

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
#define SPM_META_DESCRIPTOR ".SPM_DESCRIPTOR"
#define SPM_META_FILELIST ".SPM_FILELIST"
#define SPM_META_PREFIX_PLACEHOLDER \
"_0________________________________________________\
_1________________________________________________"

// GLOBALS
spm_vars SPM_GLOBAL;

#ifdef __APPLE__
extern char **environ;
#define __environ environ
#endif

#endif //SPM_SPM_H
