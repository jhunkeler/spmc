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

/**
 * Write data to a file
 * @param filename name of file
 * @param data data to write
 * @param size type of data
 * @param nelem number of elements of `size` to write
 * @return bytes written (0 = failure)
 */
size_t mock(const char *filename, void *data, size_t size, size_t nelem) {
    unsigned long written = 0;
    FILE *fp = fopen(filename, "w+b");
    if (fp == NULL) {
        perror(filename);
        exit(errno);
    }
    if ((written = fwrite(data, size, nelem, fp)) == 0) {
        fprintf(stderr, "warning: %s: no data written\n", filename);
    }
    fclose(fp);
    return written;
}

/**
 * Generate a temporary file of `size` in bytes, filled with `fill_byte`s
 * @param size size of new file in bytes
 * @param fill_byte byte to write `size` times
 * @return temporary file path
 */
char *mock_size(size_t size, const char *fill_byte) {
    FILE *fp = NULL;
    char *buffer = NULL;
    char filename[PATH_MAX] = {"mock_file_of_size.XXXXXX"};

    if ((buffer = malloc(size)) == NULL) {
        perror("unable to allocate buffer");
        exit(errno);
    }

    if (fill_byte == NULL) {
        return NULL;
    }

    if ((mkstemp(filename)) < 0) {
        perror("mktemp failed to create temporary file");
        exit(errno);
    }

    if ((fp = fopen(filename, "w+")) == NULL) {
        perror(filename);
        exit(errno);
    }

    memset(buffer, fill_byte[0], size);
    size_t bytes = fwrite(buffer, sizeof(char), size, fp);
    if (bytes == 0 && ferror(fp) != 0) {
        fprintf(stderr, "%s: read failed\n", filename);
        exit(1);
    }

    fclose(fp);
    free(buffer);

    return strdup(filename);
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
