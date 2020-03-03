/**
 * @file strings.c
 */
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
 * @return 1 = found, 0 = not found, -1 = error
 */
int startswith(const char *sptr, const char *pattern) {
    if (!sptr) {
        return -1;
    }
    for (size_t i = 0; i < strlen(pattern); i++) {
        if (sptr[i] != pattern[i]) {
            return 0;
        }
    }
    return 1;
}

/**
 * Scan for `pattern` string at the end of `sptr`
 *
 * @param sptr string to scan
 * @param pattern string to search for
 * @return 1 = found, 0 = not found, -1 = error
 */
int endswith(const char *sptr, const char *pattern) {
    if (!sptr) {
        return -1;
    }
    size_t sptr_size = strlen(sptr);
    size_t pattern_size = strlen(pattern);
    for (size_t s = sptr_size - pattern_size, p = 0 ; s < sptr_size; s++, p++) {
        if (sptr[s] != pattern[p]) {
            // sptr does not end with pattern
            return 0;
        }
    }
    // sptr ends with pattern
    return 1;
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
    if (_sptr == NULL) {
        return NULL;
    }
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
        result[i] = (char *)calloc(strlen(token) + 1, sizeof(char));
        if (!result[i]) {
            free(sptr);
            return NULL;
        }
        memcpy(result[i], token, strlen(token) + 1);   // copy the string contents into the record
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
 * Create new a string from an array of strings
 *
 * ~~~{.c}
 * char *array[] = {
 *     "this",
 *     "is",
 *     "a",
 *     "test",
 *     NULL,
 * }
 *
 * char *test = join(array, " ");    // "this is a test"
 * char *test2 = join(array, "_");   // "this_is_a_test"
 * char *test3 = join(array, ", ");  // "this, is, a, test"
 *
 * free(test);
 * free(test2);
 * free(test3);
 * ~~~
 *
 * @param arr
 * @param separator characters to insert between elements in string
 * @return new joined string
 */
char *join(char **arr, const char *separator) {
    char *result = NULL;
    int records = 0;
    size_t total_bytes = 0;

    if (!arr) {
        return NULL;
    }

    for (int i = 0; arr[i] != NULL; i++) {
        total_bytes += strlen(arr[i]);
        records++;
    }
    total_bytes += (records * strlen(separator)) + 1;

    result = (char *)calloc(total_bytes, sizeof(char));
    for (int i = 0; i < records; i++) {
        strcat(result, arr[i]);
        if (i < (records - 1)) {
            strcat(result, separator);
        }
    }
    return result;
}

/**
 * Join two or more strings by a `separator` string
 * @param separator
 * @param ...
 * @return string
 */
char *join_ex(char *separator, ...) {
    va_list ap;                 // Variadic argument list
    size_t separator_len = 0;   // Length of separator string
    size_t size = 0;            // Length of output string
    size_t argc = 0;            // Number of arguments ^ "..."
    char **argv = NULL;         // Arguments
    char *current = NULL;       // Current argument
    char *result = NULL;        // Output string

    // Initialize array
    argv = calloc(argc + 1, sizeof(char *));
    if (argv == NULL) {
        perror("join_ex calloc failed");
        return NULL;
    }

    // Get length of the separator
    separator_len = strlen(separator);

    // Process variadic arguments:
    // 1. Iterate over argument list `ap`
    // 2. Assign `current` with the value of argument in `ap`
    // 3. Extend the `argv` array by the latest argument count `argc`
    // 4. Sum the length of the argument and the `separator` passed to the function
    // 5. Append `current` string to `argv` array
    // 6. Update argument counter `argc`
    va_start(ap, separator);
    for(argc = 0; (current = va_arg(ap, char *)) != NULL; argc++) {
        char **tmp = realloc(argv, (argc + 1) * sizeof(char *));
        if (tmp == NULL) {
            perror("join_ex realloc failed");
            return NULL;
        }
        argv = tmp;
        size += strlen(current) + separator_len;
        argv[argc] = strdup(current);
    }
    va_end(ap);

    // Generate output string
    result = calloc(size + 1, sizeof(char));
    for (size_t i = 0; i < argc; i++) {
        // Append argument to string
        strcat(result, argv[i]);

        // Do not append a trailing separator when we reach the last argument
        if (i < (argc - 1)) {
            strcat(result, separator);
        }
        free(argv[i]);
    }
    free(argv);

    return result;
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
 * @return yes=`pointer to string`, no=`NULL`, failure=`NULL`
 */
char *strstr_array(char **arr, const char *str) {
    if (arr == NULL) {
        return NULL;
    }

    for (int i = 0; arr[i] != NULL; i++) {
        if (strstr(arr[i], str) != NULL) {
            return arr[i];
        }
    }
    return NULL;
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

    size_t records;
    // Determine the length of the array
    for (records = 0; arr[records] != NULL; records++);

    // Allocate enough memory to store the original array contents
    // (It might not have duplicate values, for example)
    char **result = (char **)calloc(records + 1, sizeof(char *));
    if (!result) {
        return NULL;
    }

    int rec = 0;
    size_t i = 0;
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
        memcpy(result[rec], arr[i], strlen(arr[i]) + 1);
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
    while (isblank(*tmp) || isspace(*tmp)) {
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
    size_t len = strlen(sptr);
    if (len == 0) {
        return sptr;
    }
    else if (len == 1 && (isblank(*sptr) || isspace(*sptr))) {
        *sptr = '\0';
        return sptr;
    }
    for (size_t i = len; i != 0; --i) {
        if (sptr[i] == '\0') {
            continue;
        }
        if (isspace(sptr[i]) || isblank(sptr[i])) {
            sptr[i] = '\0';
        }
        else {
            break;
        }
    }
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
        if (!isblank(*tmp) || !isspace(*tmp)) {
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

/**
 * Determine whether the input character is a relational operator
 * Note: `~` is non-standard
 * @param ch
 * @return 0=no, 1=yes
 */
int isrelational(char ch) {
    char symbols[] = "~!=<>";
    char *symbol = symbols;
    while (*symbol != '\0') {
        if (ch == *symbol) {
            return 1;
        }
        symbol++;
    }
    return 0;
}

/**
 * Print characters in `s`, `len` times
 * @param s
 * @param len
 */
void print_banner(const char *s, int len) {
    size_t s_len = strlen(s);
    if (!s_len) {
        return;
    }
    for (size_t i = 0; i < (len / s_len); i++) {
        for (size_t c = 0; c < s_len; c++) {
            putchar(s[c]);
        }
    }
    putchar('\n');
}

/**
 * Collapse whitespace in `s`. The string is modified in place.
 * @param s
 * @return pointer to `s`
 */
char *normalize_space(char *s) {
    size_t len;
    size_t trim_pos;
    int add_whitespace = 0;
    char *result = s;
    char *tmp;
    if ((tmp = calloc(strlen(s) + 1, sizeof(char))) == NULL) {
        perror("could not allocate memory for temporary string");
        return NULL;
    }
    char *tmp_orig = tmp;

    // count whitespace, if any
    for (trim_pos = 0; isblank(s[trim_pos]); trim_pos++);
    // trim whitespace from the left, if any
    memmove(s, &s[trim_pos], strlen(&s[trim_pos]));
    // cull bytes not part of the string after moving
    len = strlen(s);
    s[len - trim_pos] = '\0';

    // Generate a new string with extra whitespace stripped out
    while (*s != '\0') {
        // Skip over any whitespace, but record that we encountered it
        if (isblank(*s)) {
            s++;
            add_whitespace = 1;
            continue;
        }
        // This gate avoids filling tmp with whitespace; we want to make our own
        if (add_whitespace) {
            *tmp = ' ';
            tmp++;
            add_whitespace = 0;
        }
        // Write character in s to tmp
        *tmp = *s;
        // Increment string pointers
        s++;
        tmp++;
    }

    // Rewrite the input string
    strcpy(result, tmp_orig);
    free(tmp_orig);
    return result;
}

/**
 * Duplicate an array of strings
 * @param array
 * @return
 */
char **strdup_array(char **array) {
    char **result = NULL;
    size_t elems = 0;

    // Guard
    if (array == NULL) {
        return NULL;
    }

    // Count elements in `array`
    for (elems = 0; array[elems] != NULL; elems++);

    // Create new array
    result = calloc(elems + 1, sizeof(char *));
    for (size_t i = 0; i < elems; i++) {
        result[i] = strdup(array[i]);
    }

    return result;
}
