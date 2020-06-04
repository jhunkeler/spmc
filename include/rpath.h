/**
 * @file rpath.h
 */
#ifndef SPM_RPATH_H
#define SPM_RPATH_H

Process *patchelf(const char *_filename, const char *_args);
Process *install_name_tool(const char *_filename, const char *_args);
FSTree *rpath_libraries_available(const char *root);
char *rpath_autodetect(const char *filename, FSTree *tree, const char *destroot);
int has_rpath(const char *_filename);
char *rpath_get(const char *_filename);
char *rpath_generate(const char *_filename, FSTree *tree, const char *destroot);
int rpath_autoset(const char *filename, FSTree *tree, const char *destroot);
int rpath_set(const char *filename, const char *rpath);

#endif //SPM_RPATH_H
