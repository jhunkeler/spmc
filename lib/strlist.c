/**
 * String array convenience functions
 * @file strlist.c
 */
#include "spm.h"
#include "strlist.h"
#include "url.h"

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

static int reader_strlist_append_file(size_t lineno, char **line) {
    (void)(lineno);  // unused parameter
    (void)(line);   // unused parameter
    return 0;
}

/**
 * Append lines from a local file or remote URL (HTTP/s only)
 * @param pStrList
 * @param path file path or HTTP/s address
 * @param readerFn pointer to a reader function (use NULL to retrieve all data)
 * @return 0=success 1=no data, -1=error (spmerrno set)
 */
int strlist_append_file(StrList *pStrList, char *_path, ReaderFn *readerFn) {
    int retval = 0;
    int is_remote = 0;
    char *path = NULL;
    char *filename = NULL;
    char *from_file_tmpdir = NULL;
    char **data = NULL;

    if (readerFn == NULL) {
        readerFn = reader_strlist_append_file;
    }

    path = strdup(_path);
    if (path == NULL) {
        spmerrno = errno;
        retval = -1;
        goto fatal;
    }

    is_remote = (startswith(path, "http") || startswith(path, "https"));
    if (is_remote) {
        long response = 0;
        char *fetched = NULL;
        from_file_tmpdir = spm_mkdtemp(TMP_DIR, __FUNCTION__, NULL);

        if (from_file_tmpdir == NULL) {
            spmerrno = errno;
            spmerrno_cause("file_from temp directory");
            retval = -1;
            goto fatal;
        }

        fetched = join((char *[]){from_file_tmpdir, basename(path), NULL}, DIRSEPS);
        if ((response = fetch(path, fetched)) >= 400) {
            spmerrno = SPM_ERR_FETCH;
            char cause[PATH_MAX];
            sprintf(cause, "HTTP(%ld): %s: %s", response, http_response_str(response), path);
            spmerrno_cause(cause);
            retval = -1;
            goto fatal;
        }

        filename = strdup(fetched);
        free(fetched);
    } else {
        filename = expandpath(path);
    }

    if (filename == NULL) {
        spmerrno = errno;
        retval = -1;
        goto fatal;
    }

    data = file_readlines(filename, 0, 0, readerFn);
    if (data == NULL) {
        retval = 1;
        goto fatal;
    }

    for (size_t record = 0; data[record] != NULL; record++) {
        strlist_append(pStrList, data[record]);
        free(data[record]);
    }
    free(data);

fatal:
    if (from_file_tmpdir != NULL) {
        rmdirs(from_file_tmpdir);
        free(from_file_tmpdir);
    }
    if (filename != NULL) {
        free(filename);
    }
    if (path != NULL) {
        free(path);
    }

    return retval;
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
 * Produce a new copy of a `StrList`
 * @param pStrList  `StrList`
 * @return `StrList` copy
 */
StrList *strlist_copy(StrList *pStrList) {
    StrList *result = strlist_init();
    if (pStrList == NULL || result == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < strlist_count(pStrList); i++) {
        strlist_append(result, strlist_item(pStrList, i));
    }
    return result;
}

/**
 * Remove a record by index from a `StrList`
 * @param pStrList
 * @param index
 */
void strlist_remove(StrList *pStrList, size_t index) {
    size_t count = strlist_count(pStrList);
    if (count == 0) {
        return;
    }

    for (size_t i = index; i < count; i++) {
        char *next = pStrList->data[i + 1];
        pStrList->data[i] = next;
        if (next == NULL) {
            break;
        }
    }

    pStrList->num_inuse--;
}

/**
 * Compare two `StrList`s
 * @param a `StrList` structure
 * @param b `StrList` structure
 * @return same=0, different=1, error=-1 (a is NULL), -2 (b is NULL)
 */
int strlist_cmp(StrList *a, StrList *b) {
    if (a == NULL) {
        return -1;
    }

    if (b == NULL) {
        return -2;
    }

    if (a->num_alloc != b->num_alloc) {
        return 1;
    }

    if (a->num_inuse != b->num_inuse) {
        return 1;
    }

    for (size_t i = 0; i < strlist_count(a); i++) {
        if (strcmp(strlist_item(a, i), strlist_item(b, i)) != 0) {
            return 1;
        }
    }

    return 0;
}

/**
 * Sort a `StrList` by `mode`
 * @param pStrList
 * @param mode Available modes: `STRLIST_DEFAULT` (alphabetic), `STRLIST_ASC` (ascending), `STRLIST_DSC` (descending)
 */
void strlist_sort(StrList *pStrList, unsigned int mode) {
    void *fn = NULL;

    if (pStrList == NULL) {
        return;
    }

    strsort(pStrList->data, mode);
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
    return (char) strtol(strlist_item(pStrList, index), NULL, 10);
}

/**
 * Convert value at index to `unsigned char`
 * @param pStrList
 * @param index
 * @return `unsigned char`
 */
unsigned char strlist_item_as_uchar(StrList *pStrList, size_t index) {
    return (unsigned char) strtol(strlist_item(pStrList, index), NULL, 10);
}

/**
 * Convert value at index to `short`
 * @param pStrList
 * @param index
 * @return `short`
 */
short strlist_item_as_short(StrList *pStrList, size_t index) {
    return (short)strtol(strlist_item(pStrList, index), NULL, 10);
}

/**
 * Convert value at index to `unsigned short`
 * @param pStrList
 * @param index
 * @return `unsigned short`
 */
unsigned short strlist_item_as_ushort(StrList *pStrList, size_t index) {
    return (unsigned short)strtoul(strlist_item(pStrList, index), NULL, 10);
}

/**
 * Convert value at index to `int`
 * @param pStrList
 * @param index
 * @return `int`
 */
int strlist_item_as_int(StrList *pStrList, size_t index) {
    return (int)strtol(strlist_item(pStrList, index), NULL, 10);
}

/**
 * Convert value at index to `unsigned int`
 * @param pStrList
 * @param index
 * @return `unsigned int`
 */
unsigned int strlist_item_as_uint(StrList *pStrList, size_t index) {
    return (unsigned int)strtoul(strlist_item(pStrList, index), NULL, 10);
}

/**
 * Convert value at index to `long`
 * @param pStrList
 * @param index
 * @return `long`
 */
long strlist_item_as_long(StrList *pStrList, size_t index) {
    return strtol(strlist_item(pStrList, index), NULL, 10);
}

/**
 * Convert value at index to `unsigned long`
 * @param pStrList
 * @param index
 * @return `unsigned long`
 */
unsigned long strlist_item_as_ulong(StrList *pStrList, size_t index) {
    return strtoul(strlist_item(pStrList, index), NULL, 10);
}

/**
 * Convert value at index to `long long`
 * @param pStrList
 * @param index
 * @return `long long`
 */
long long strlist_item_as_long_long(StrList *pStrList, size_t index) {
    return strtoll(strlist_item(pStrList, index), NULL, 10);
}

/**
 * Convert value at index to `unsigned long long`
 * @param pStrList
 * @param index
 * @return `unsigned long long`
 */
unsigned long long strlist_item_as_ulong_long(StrList *pStrList, size_t index) {
    return strtoull(strlist_item(pStrList, index), NULL, 10);
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
