/**
 * @file shlib.h
 */
#ifndef SPM_SHLIB_H
#define SPM_SHLIB_H

#if OS_WINDOWS && defined(_MSC_VER)
#define SPM_SHLIB_EXEC "dumpbin"
#define SPM_SHLIB_EXEC_ARGS "/dependents"
#define SPM_SHLIB_EXTENSION ".dll"
#elif OS_DARWIN
#define SPM_SHLIB_EXEC "/usr/bin/objdump"
#define SPM_SHLIB_EXEC_ARGS "-macho -p"
#define SPM_SHLIB_EXTENSION ".dylib"
#else  // linux (hopefully)
#define SPM_SHLIB_EXEC "/usr/bin/objdump"
#define SPM_SHLIB_EXEC_ARGS "-p"
#define SPM_SHLIB_EXTENSION ".so"
#endif

char *objdump(const char *_filename, char *_args);
char *shlib_rpath(const char *filename);
StrList *shlib_deps(const char *_filename);

#endif //SPM_SHLIB_H
