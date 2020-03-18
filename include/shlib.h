/**
 * @file shlib.h
 */
#ifndef SPM_SHLIB_H
#define SPM_SHLIB_H

#if defined(_MSC_VER)
#define SPM_SHLIB_EXEC "dumpbin"
#define SPM_SHLIB_EXEC_ARGS "/dependents"
#define SPM_SHLIB_EXTENSION ".dll"
#elif defined(__APPLE__) && defined(__MACH__)
#define SPM_SHLIB_EXEC "/usr/bin/otool"
#define SPM_SHLIB_EXEC_ARGS "-l"
#define SPM_SHLIB_EXTENSION ".dylib"
#else  // linux (hopefully)
#define SPM_SHLIB_EXEC "/usr/bin/objdump"
#define SPM_SHLIB_EXEC_ARGS "-p"
#define SPM_SHLIB_EXTENSION ".so"
#endif

StrList *shlib_deps(const char *_filename);

#endif //SPM_SHLIB_H
