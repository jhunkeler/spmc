/**
 * String array convenience functions
 * @file strlist.h
 */
#ifndef SPM_STRLIST_H
#define SPM_STRLIST_H
#include "metadata.h"

typedef struct {
    size_t num_alloc;
    size_t num_inuse;
    char **data;
} StrList;

StrList *strlist_init();
void strlist_remove(StrList *pStrList, size_t index);
long double strlist_item_as_long_double(StrList *pStrList, size_t index);
double strlist_item_as_double(StrList *pStrList, size_t index);
float strlist_item_as_float(StrList *pStrList, size_t index);
unsigned long long strlist_item_as_ulong_long(StrList *pStrList, size_t index);
long long strlist_item_as_long_long(StrList *pStrList, size_t index);
unsigned long strlist_item_as_ulong(StrList *pStrList, size_t index);
long strlist_item_as_long(StrList *pStrList, size_t index);
unsigned int strlist_item_as_uint(StrList *pStrList, size_t index);
int strlist_item_as_int(StrList *pStrList, size_t index);
unsigned short strlist_item_as_ushort(StrList *pStrList, size_t index);
short strlist_item_as_short(StrList *pStrList, size_t index);
unsigned char strlist_item_as_uchar(StrList *pStrList, size_t index);
char strlist_item_as_char(StrList *pStrList, size_t index);
char *strlist_item_as_str(StrList *pStrList, size_t index);
char *strlist_item(StrList *pStrList, size_t index);
void strlist_set(StrList *pStrList, size_t index, char *value);
size_t strlist_count(StrList *pStrList);
void strlist_reverse(StrList *pStrList);
void strlist_sort(StrList *pStrList, unsigned int mode);
int strlist_append_file(StrList *pStrList, char *path, ReaderFn *readerFn);
void strlist_append_strlist(StrList *pStrList1, StrList *pStrList2);
void strlist_append(StrList *pStrList, char *str);
StrList *strlist_copy(StrList *pStrList);
int strlist_cmp(StrList *a, StrList *b);
void strlist_free(StrList *pStrList);

#endif //SPM_STRLIST_H
