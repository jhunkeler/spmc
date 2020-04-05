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
 * Hexdump a character array to stdout
 * @param addr starting address
 * @param size size to read
 */
void hexdump(char *addr, size_t size) {
    char buf[32];
    for (size_t i = 0, char_count = 0; i <= size; i++, char_count++) {
        if (i % 16 == 0) {
            printf("%04X: ", (unsigned int)i);
        }

        printf("%02X ", (unsigned char)addr[i]);
        if (i % 16 == 15) {
            printf("|%-16s|\n", buf);
            char_count = 0;
            memset(buf, '\0', sizeof(buf));
        }

        if (!isprint(addr[i])) {
            buf[char_count] = '.';
        } else {
            buf[char_count] = addr[i];
        }
    }
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

#define myassert(condition, ...) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "%s:%d:%s :: ", __FILE__, __LINE__, __FUNCTION__); \
            fprintf(stderr, __VA_ARGS__); \
            return 1; \
        } \
    } while (0)

#endif //SPM_FRAMEWORK_H
