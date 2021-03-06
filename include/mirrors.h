/**
 * @file mirrors.h
 */
#ifndef SPM_MIRRORS_H
#define SPM_MIRRORS_H

#define SPM_MIRROR_MAX 0xff
#define SPM_MIRROR_FILENAME "mirrorlist"

char **mirror_list(const char *filename);
void mirror_list_free(char **m);
int mirror_clone(Manifest *info, char *_dest);

#endif //SPM_MIRRORS_H
