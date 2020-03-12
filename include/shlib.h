#ifndef SPM_SHLIB_H
#define SPM_SHLIB_H

#ifdef __APPLE__
#define SPM_SHLIB_EXEC "/usr/bin/otool"
#define SPM_SHLIB_EXEC_ARGS "-l"
#else
#define SPM_SHLIB_EXEC "/usr/bin/objdump"
#define SPM_SHLIB_EXEC_ARGS "-p"
#endif

StrList *shlib_deps(const char *_filename);

#endif //SPM_SHLIB_H
