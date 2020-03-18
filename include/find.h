/**
 * @file find.h
 */
#ifndef SPM_FIND_H
#define SPM_FIND_H

char *find_executable(const char *program);
char *find_file(const char *root, const char *filename);
char *find_package(const char *filename);
int errglob(const char *epath, int eerrno);

#endif //SPM_FIND_H
