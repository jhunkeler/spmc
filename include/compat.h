#ifndef SPM_COMPAT_H
#define SPM_COMPAT_H

#include "config.h"

#ifndef HAVE_STRSEP
char *strsep(char **sp, char *sep);
#endif

#ifndef HAVE_REALLOC_ARRAY
void *reallocarray (void *__ptr, size_t __nmemb, size_t __size);
#endif

#endif //SPM_COMPAT_H
