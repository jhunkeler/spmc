#ifndef SPM_RPATH_H
#define SPM_RPATH_H

Process *patchelf(const char *_filename, const char *_args);
char *rpath_autodetect(const char *filename);
int has_rpath(const char *_filename);
char *rpath_get(const char *_filename);
char *rpath_generate(const char *_filename);
int rpath_autoset(const char *filename);
int rpath_set(const char *filename, const char *rpath);

#endif //SPM_RPATH_H
