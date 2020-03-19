#include "config.h"

#ifndef HAVE_STRSEP
#include <string.h>
// credit: Dan Cross via https://unixpapa.com/incnote/string.html
char *strsep(char **sp, char *sep)
{
    char *p, *s;
    if (sp == NULL || *sp == NULL || **sp == '\0') return(NULL);
    s = *sp;
    p = s + strcspn(s, sep);
    if (*p != '\0') *p++ = '\0';
    *sp = p;
    return(s);
}
#endif

#ifndef HAVE_REALLOCARRAY
#include <stdlib.h>
void *reallocarray (void *__ptr, size_t __nmemb, size_t __size) {
    return realloc(__ptr, __nmemb * __size);
}
#endif
