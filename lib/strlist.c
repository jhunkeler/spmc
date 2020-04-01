/**
 * String array convenience functions
 * @file strlist.c
 */
#include "spm.h"
#include "strlist.h"

/**
 *
 * @param pStrList `StrList`
 */
void strlist_free(StrList *pStrList) {
    if (pStrList == NULL) {
        return;
    }
    for (size_t i = 0; i < pStrList->num_inuse; i++) {
        free(pStrList->data[i]);
    }
    free(pStrList->data);
    free(pStrList);
}

/**
 * Append a value to the list
 * @param pStrList `StrList`
 * @param str
 */
void strlist_append(StrList *pStrList, char *str) {
    char **tmp = NULL;

    if (pStrList == NULL) {
        return;
    }

    tmp = realloc(pStrList->data, (pStrList->num_alloc + 1) * sizeof(char *));
    if (tmp == NULL) {
        strlist_free(pStrList);
        perror("failed to append to array");
        exit(1);
    }
    pStrList->data = tmp;
    pStrList->data[pStrList->num_inuse] = strdup(str);
    pStrList->data[pStrList->num_alloc] = NULL;
    strcpy(pStrList->data[pStrList->num_inuse], str);
    pStrList->num_inuse++;
    pStrList->num_alloc++;
}

/**
 * Append the contents of a `StrList` to another `StrList`
 * @param pStrList1 `StrList`
 * @param pStrList2 `StrList`
 */
void strlist_append_strlist(StrList *pStrList1, StrList *pStrList2) {
    size_t count = 0;

    if (pStrList1 == NULL || pStrList2 == NULL) {
        return;
    }

    count = strlist_count(pStrList2);
    for (size_t i = 0; i < count; i++) {
        char *item = strlist_item(pStrList2, i);
        strlist_append(pStrList1, item);
    }
}

/**
 *
 * @param a
 * @param b
 * @return
 */
static int _strlist_cmpfn(const void *a, const void *b) {
    const char *aa = *(const char**)a;
    const char *bb = *(const char**)b;
    int result = strcmp(aa, bb);
    return result;
}

/**
 *
 * @param a
 * @param b
 * @return
 */
static int _strlist_asc_cmpfn(const void *a, const void *b) {
    const char *aa = *(const char**)a;
    const char *bb = *(const char**)b;
    size_t len_a = strlen(aa);
    size_t len_b = strlen(bb);
    return len_a > len_b - strcmp(aa, bb);
}

/**
 *
 * @param a
 * @param b
 * @return
 */
static int _strlist_dsc_cmpfn(const void *a, const void *b) {
    const char *aa = *(const char**)a;
    const char *bb = *(const char**)b;
    size_t len_a = strlen(aa);
    size_t len_b = strlen(bb);
    return len_a < len_b - strcmp(aa, bb);
}

/**
 * Sort a `StrList` by `mode`
 * @param pStrList
 * @param mode Available modes: `STRLIST_DEFAULT` (alphabetic), `STRLIST_ASC` (ascending), `STRLIST_DSC` (descending)
 */
void strlist_sort(StrList *pStrList, unsigned int mode) {
    // TODO: use strsort_array() instead instead of duplicating effort
    void *fn = NULL;

    if (pStrList == NULL) {
        return;
    }

    switch (mode) {
        case STRLIST_ASC:
            fn = _strlist_asc_cmpfn;
            break;
        case STRLIST_DSC:
            fn = _strlist_dsc_cmpfn;
            break;
        case STRLIST_DEFAULT:
        default:
            fn = _strlist_cmpfn;
            break;
    }

    qsort(pStrList->data, pStrList->num_inuse, sizeof(char *), fn);
}

/**
 * Reverse the order of a `StrList`
 * @param pStrList
 */
void strlist_reverse(StrList *pStrList) {
    char *tmp = NULL;
    size_t i = 0;
    size_t j = 0;

    if (pStrList == NULL) {
        return;
    }

    j = pStrList->num_inuse - 1;
    for (i = 0; i < j; i++) {
        tmp = pStrList->data[i];
        pStrList->data[i] = pStrList->data[j];
        pStrList->data[j] = tmp;
        j--;
    }
}

/**
 * Get the count of values stored in a `StrList`
 * @param pStrList
 * @return
 */
size_t strlist_count(StrList *pStrList) {
    return pStrList->num_inuse;
}

/**
 * Set value at index
 * @param pStrList
 * @param value string
 * @return
 */
void strlist_set(StrList *pStrList, size_t index, char *value) {
    char *tmp = NULL;
    char *item = NULL;
    if (pStrList == NULL || index > strlist_count(pStrList)) {
        return;
    }
    if ((item = strlist_item(pStrList, index)) == NULL) {
        return;
    }
    if (value == NULL) {
        pStrList->data[index] = NULL;
    } else {
        if ((tmp = realloc(pStrList->data[index], strlen(value) + 1)) == NULL) {
            perror("realloc strlist_set replacement value");
            return;
        }

        pStrList->data[index] = tmp;
        memset(pStrList->data[index], '\0', strlen(value) + 1);
        strncpy(pStrList->data[index], value, strlen(value));
    }
}

/**
 * Retrieve data from a `StrList`
 * @param pStrList
 * @param index
 * @return string
 */
char *strlist_item(StrList *pStrList, size_t index) {
    if (pStrList == NULL || index > strlist_count(pStrList)) {
        return NULL;
    }
    return pStrList->data[index];
}

/**
 * Alias of `strlist_item`
 * @param pStrList
 * @param index
 * @return string
 */
char *strlist_item_as_str(StrList *pStrList, size_t index) {
    return strlist_item(pStrList, index);
}

/**
 * Convert value at index to `char`
 * @param pStrList
 * @param index
 * @return `char`
 */
char strlist_item_as_char(StrList *pStrList, size_t index) {
    return (char) *(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `unsigned char`
 * @param pStrList
 * @param index
 * @return `unsigned char`
 */
unsigned char strlist_item_as_uchar(StrList *pStrList, size_t index) {
    return (unsigned char) *(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `short`
 * @param pStrList
 * @param index
 * @return `short`
 */
short strlist_item_as_short(StrList *pStrList, size_t index) {
    return (short)atoi(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `unsigned short`
 * @param pStrList
 * @param index
 * @return `unsigned short`
 */
unsigned short strlist_item_as_ushort(StrList *pStrList, size_t index) {
    return (unsigned short)atoi(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `int`
 * @param pStrList
 * @param index
 * @return `int`
 */
int strlist_item_as_int(StrList *pStrList, size_t index) {
    return atoi(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `unsigned int`
 * @param pStrList
 * @param index
 * @return `unsigned int`
 */
unsigned int strlist_item_as_uint(StrList *pStrList, size_t index) {
    return (unsigned int)atoi(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `long`
 * @param pStrList
 * @param index
 * @return `long`
 */
long strlist_item_as_long(StrList *pStrList, size_t index) {
    return atol(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `unsigned long`
 * @param pStrList
 * @param index
 * @return `unsigned long`
 */
unsigned long strlist_item_as_ulong(StrList *pStrList, size_t index) {
    return (unsigned long)atol(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `long long`
 * @param pStrList
 * @param index
 * @return `long long`
 */
long long strlist_item_as_long_long(StrList *pStrList, size_t index) {
    return (long long)atoll(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `unsigned long long`
 * @param pStrList
 * @param index
 * @return `unsigned long long`
 */
unsigned long long strlist_item_as_ulong_long(StrList *pStrList, size_t index) {
    return (unsigned long long)atoll(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `float`
 * @param pStrList
 * @param index
 * @return `float`
 */
float strlist_item_as_float(StrList *pStrList, size_t index) {
    return (float)atof(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `double`
 * @param pStrList
 * @param index
 * @return `double`
 */
double strlist_item_as_double(StrList *pStrList, size_t index) {
    return atof(strlist_item(pStrList, index));
}

/**
 * Convert value at index to `long double`
 * @param pStrList
 * @param index
 * @return `long double`
 */
long double strlist_item_as_long_double(StrList *pStrList, size_t index) {
    return (long double)atof(strlist_item(pStrList, index));
}

/**
 * Initialize an empty `StrList`
 * @return `StrList`
 */
StrList *strlist_init() {
    StrList *pStrList = calloc(1, sizeof(StrList));
    if (pStrList == NULL) {
        perror("failed to allocate array");
        exit(errno);
    }
    pStrList->num_inuse = 0;
    pStrList->num_alloc = 1;
    pStrList->data = calloc(pStrList->num_alloc, sizeof(char *));
    return pStrList;
}
