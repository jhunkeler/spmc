#ifndef SPM_FRAMEWORK_H
#define SPM_FRAMEWORK_H
#include <limits.h>
#include <fcntl.h>

union TestValue {
    const char *sptr;
    char **slptr;
    const char **cslptr;
    char character;
    unsigned int unsigned_integer;
    signed int signed_integer;
    float floating;
    char str[PATH_MAX];
};

struct TestCase {
    union TestValue caseValue;
    union TestValue inputValue;
    union TestValue truthValue;
    union TestValue arg[10]; // when there are too many damn arguments
};

char *array_to_string(char **array, const char *sep) {
    char *buffer = NULL;
    size_t buffer_size = 0;
    size_t records = 0;

    if (array == NULL || sep == NULL) {
        return NULL;
    }

    for (records = 0; array[records] != NULL; records++) {
        buffer_size += strlen(array[records]) + 1;
    }
    buffer_size++;

    buffer = calloc(buffer_size, sizeof(char));
    if (buffer == NULL) {
        perror("could not allocate buffer");
        exit(1);
    }

    for (size_t i = 0; i < records; i++) {
        for (size_t ch = 0; ch < strlen(array[i]); ch++) {
            strncat(buffer, &array[i][ch], 1);
        }
        if ((records - (i + 1)) > 0)
            strncat(buffer, sep, 2);
    }
    return buffer;
}

#define myassert(condition, ...) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "%s:%d:%s :: ", __FILE__, __LINE__, __FUNCTION__); \
            fprintf(stderr, __VA_ARGS__); \
            return 1; \
        } \
    } while (0)

#endif //SPM_FRAMEWORK_H
