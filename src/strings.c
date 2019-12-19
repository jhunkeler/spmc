#include "spm.h"

/**
 * Determine how many times the character `ch` appears in `sptr` string
 * @param sptr string to scan
 * @param ch character to find
 * @return count of characters found
 */
int num_chars(const char *sptr, int ch) {
    int result = 0;
    for (int i = 0; sptr[i] != '\0'; i++) {
        if (sptr[i] == ch) {
            result++;
        }
    }
    return result;
}

/**
 * Scan for `pattern` string at the beginning of `sptr`
 *
 * @param sptr string to scan
 * @param pattern string to search for
 * @return 0 = found, 1 = not found, -1 = error
 */
int startswith(const char *sptr, const char *pattern) {
    if (!sptr) {
        return -1;
    }
    for (size_t i = 0; i < strlen(pattern); i++) {
        if (sptr[i] != pattern[i]) {
            return 1;
        }
    }
    return 0;
}

/**
 * Scan for `pattern` string at the end of `sptr`
 *
 * @param sptr string to scan
 * @param pattern string to search for
 * @return 0 = found, 1 = not found, -1 = error
 */
int endswith(const char *sptr, const char *pattern) {
    if (!sptr) {
        return -1;
    }
    size_t sptr_size = strlen(sptr);
    size_t pattern_size = strlen(pattern);
    for (size_t s = sptr_size - pattern_size, p = 0 ; s < sptr_size; s++, p++) {
        if (sptr[s] != pattern[p]) {
            return 1;
        }
    }
    return 0;
}

/**
 * Deletes any characters matching `chars` from `sptr` string
 *
 * @param sptr string to be modified in-place
 * @param chars a string containing characters (e.g. " \n" would delete whitespace and line feeds)
 */
void strchrdel(char *sptr, const char *chars) {
    while (*sptr != '\0') {
        for (int i = 0; chars[i] != '\0'; i++) {
            if (*sptr == chars[i]) {
                memmove(sptr, sptr + 1, strlen(sptr));
            }
        }
        sptr++;
    }
}

/**
 * Find the integer offset of the first occurrence of `ch` in `sptr`
 *
 * ~~~{.c}
 * char buffer[255];
 * char string[] = "abc=123";
 * long int separator_offset = strchroff(string, '=');
 * for (long int i = 0; i < separator_offset); i++) {
 *     buffer[i] = string[i];
 * }
 * ~~~
 *
 * @param sptr string to scan
 * @param ch character to find
 * @return offset to character in string, or 0 on failure
 */
long int strchroff(const char *sptr, int ch) {
    char *orig = strdup(sptr);
    char *tmp = orig;
    long int result = 0;
    while (*tmp != '\0') {
        if (*tmp == ch) {
            break;
        }
        tmp++;
    }
    result = tmp - orig;
    free(orig);

    return result;
}

/**
 * This function scans `sptr` from right to left removing any matches to `suffix`
 * from the string.
 *
 * @param sptr string to be modified
 * @param suffix string to be removed from `sptr`
 */
void strdelsuffix(char *sptr, const char *suffix) {
    if (!sptr || !suffix) {
        return;
    }
    size_t sptr_len = strlen(sptr);
    size_t suffix_len = strlen(suffix);
    intptr_t target_offset = sptr_len - suffix_len;

    // Prevent access to memory below input string
    if (target_offset < 0) {
        return;
    }

    // Create a pointer to
    char *target = sptr + target_offset;
    if (!strcmp(target, suffix)) {
        // Purge the suffix
        memset(target, '\0', suffix_len);
        // Recursive call continues removing suffix until it is gone
        strip(sptr);
    }
}

/**
 * Split a string by every delimiter in `delim` string.
 *
 * Callee must free memory using `split_free()`
 *
 * @param sptr string to split
 * @param delim characters to split on
 * @return success=parts of string, failure=NULL
 */
char** split(char *_sptr, const char* delim)
{
    size_t split_alloc = 0;
    // Duplicate the input string and save a copy of the pointer to be freed later
    char *orig = strdup(_sptr);
    char *sptr = orig;
    if (!sptr) {
        return NULL;
    }

    // Determine how many delimiters are present
    for (size_t i = 0; i < strlen(delim); i++) {
        split_alloc += num_chars(sptr, delim[i]);
    }
    // Preallocate enough records based on the number of delimiters
    char **result = (char **)calloc(split_alloc + 2, sizeof(char *));
    if (!result) {
        free(sptr);
        return NULL;
    }

    // Separate the string into individual parts and store them in the result array
    int i = 0;
    char *token = NULL;
    while((token = strsep(&sptr, delim)) != NULL) {
        result[i] = (char *)calloc(1, sizeof(char) * strlen(token) + 1);
        if (!result[i]) {
            free(sptr);
            return NULL;
        }
        strncpy(result[i], token, strlen(token));   // copy the string contents into the record
        i++;    // next record
    }
    free(orig);
    return result;
}

/**
 * Frees memory allocated by `split()`
 * @param ptr pointer to array
 */
void split_free(char **ptr) {
    for (int i = 0; ptr[i] != NULL; i++) {
        free(ptr[i]);
    }
    free(ptr);
}

/**
 * Extract the string encapsulated by characters listed in `delims`
 *
 * ~~~{.c}
 * char *str = "this is [some data] in a string";
 * char *data = substring_between(string, "[]");
 * // data = "some data";
 * ~~~
 *
 * @param sptr string to parse
 * @param delims two characters surrounding a string
 * @return success=text between delimiters, failure=NULL
 */
char *substring_between(char *sptr, const char *delims) {
    // Ensure we have enough delimiters to continue
    size_t delim_count = strlen(delims);
    if (delim_count != 2) {
        return NULL;
    }

    // Create pointers to the delimiters
    char *start = strpbrk(sptr, &delims[0]);
    char *end = strpbrk(sptr, &delims[1]);

    // Ensure the string has both delimiters
    if (!start || !end) {
        return NULL;
    }

    start++;    // ignore leading delimiter

    // Get length of the substring
    size_t length = end - start;

    char *result = (char *)calloc(length + 1, sizeof(char));
    if (!result) {
        return NULL;
    }

    // Copy the contents of the substring to the result
    char *tmp = result;
    while (start != end) {
        *tmp = *start;
        tmp++;
        start++;
    }

    return result;
}

/*
 * Helper function for `strsort`
 */
static int _strsort_compare(const void *a, const void *b) {
    const char *aa = *(const char**)a;
    const char *bb = *(const char**)b;
    int result = strcmp(aa, bb);
    return result;
}

/**
 * Sort an array of strings alphabetically
 * @param arr
 */
void strsort(char **arr) {
    size_t arr_size = 0;

    // Determine size of array
    for (size_t i = 0; arr[i] != NULL; i++) {
        arr_size = i;
    }
    qsort(arr, arr_size, sizeof(char *), _strsort_compare);
}

/*
 * Helper function for `strsortlen`
 */
static int _strsortlen_asc_compare(const void *a, const void *b) {
    const char *aa = *(const char**)a;
    const char *bb = *(const char**)b;
    size_t len_a = strlen(aa);
    size_t len_b = strlen(bb);
    return len_a > len_b;
}

/*
 * Helper function for `strsortlen`
 */
static int _strsortlen_dsc_compare(const void *a, const void *b) {
    const char *aa = *(const char**)a;
    const char *bb = *(const char**)b;
    size_t len_a = strlen(aa);
    size_t len_b = strlen(bb);
    return len_a < len_b;
}
/**
 * Sort an array of strings by length
 * @param arr
 */
void strsortlen(char **arr, unsigned int sort_mode) {
    typedef int (*compar)(const void *, const void *);

    compar fn = _strsortlen_asc_compare;
    if (sort_mode != 0) {
        fn = _strsortlen_dsc_compare;
    }

    size_t arr_size = 0;

    // Determine size of array
    for (size_t i = 0; arr[i] != NULL; i++) {
        arr_size = i;
    }
    qsort(arr, arr_size, sizeof(char *), fn);
}

/**
 * Search for string in an array of strings
 * @param arr array of strings
 * @param str string to search for
 * @return yes=0, no=1, failure=-1
 */
int strstr_array(char **arr, const char *str) {
    if (!arr) {
        return -1;
    }

    for (int i = 0; arr[i] != NULL; i++) {
        if (strstr(arr[i], str) != NULL) {
            return 0;
        }
    }
    return 1;
}

/**
 * Remove duplicate strings from an array of strings
 * @param arr
 * @return success=array of unique strings, failure=NULL
 */
char **strdeldup(char **arr) {
    if (!arr) {
        return NULL;
    }

    int records;
    // Determine the length of the array
    for (records = 0; arr[records] != NULL; records++);

    // Allocate enough memory to store the original array contents
    // (It might not have duplicate values, for example)
    char **result = (char **)calloc(records + 1, sizeof(char *));
    if (!result) {
        return NULL;
    }

    int rec = 0;
    int i = 0;
    while(i < records) {
        // Search for value in results
        if (strstr_array(result, arr[i]) == 0) {
            // value already exists in results so ignore it
            i++;
            continue;
        }

        // Store unique value
        result[rec] = (char *)calloc(strlen(arr[i]) + 1, sizeof(char));
        if (!result[rec]) {
            free(result);
            return NULL;
        }
        strncpy(result[rec], arr[i], strlen(arr[i]));
        i++;
        rec++;
    }
    return result;
}

/** Remove leading whitespace from a string
 * @param sptr pointer to string
 * @return pointer to first non-whitespace character in string
 */
char *lstrip(char *sptr) {
    char *tmp = sptr;
    size_t bytes = 0;
    while (isblank(*tmp)) {
        bytes++;
        tmp++;
    }
    if (tmp != sptr) {
        memmove(sptr, sptr + bytes, strlen(sptr) - bytes);
        memset((sptr + strlen(sptr)) - bytes, '\0', bytes);
    }
    return sptr;
}

/**
 * Remove trailing whitespace from a string
 * @param sptr string
 * @return truncated string
 */
char *strip(char *sptr) {
    if (!strlen(sptr)) {
        return sptr;
    }
    strchrdel(sptr, " \r\n");
    return sptr;
}

/**
 * Determine if a string is empty
 * @param sptr pointer to string
 * @return 0=not empty, 1=empty
 */
int isempty(char *sptr) {
    char *tmp = sptr;
    while (*tmp) {
        if (!isblank(*tmp)) {
            return 0;
        }
        tmp++;
    }
    return 1;
}

/**
 * Determine if a string is encapsulated by quotes
 * @param sptr pointer to string
 * @return 0=not quoted, 1=quoted
 */
int isquoted(char *sptr) {
    const char *quotes = "'\"";
    char *quote_open = strpbrk(sptr, quotes);
    if (!quote_open) {
        return 0;
    }
    char *quote_close = strpbrk(quote_open + 1, quotes);
    if (!quote_close) {
        return 0;
    }
    return 1;
}
