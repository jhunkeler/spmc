#ifndef SPM_FRAMEWORK_H
#define SPM_FRAMEWORK_H
#include <limits.h>
#include <fcntl.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"

union TestValue {
    const char *sptr;
    char **slptr;
    const char **cslptr;
    unsigned char unsigned_char;
    char signed_char;
    unsigned short unsigned_short;
    signed short signed_short;
    unsigned int unsigned_int;
    signed int signed_int;
    unsigned long unsigned_long;
    signed long signed_long;
    unsigned long long unsigned_long_long;
    signed long long signed_long_long;
    float floating;
    void *voidptr;
    char str[PATH_MAX];
    char *strlptr[255];
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

#define AS_MOCK_LIB 0
#define AS_MOCK_BIN 1
/**
 * Create a mock binary image (shared library, or executable)
 *
 * ~~~{.c}
 * int retval;
 *
 * if ((retval = mock_image(AS_MOCK_LIB, "libwinning", NULL)) < 0) {
 *     fprintf(stderr, "Failed to create shared library\n");
 *     exit(retval);
 * }
 *
 * if ((retval = mock_image(AS_MOCK_EXEC, "winning", (char *[]){"-L.", "-lwinning", NULL}) < 0) {
 *     fprintf(stderr, "Failed to create shared library\n");
 *     exit(retval);
 * }
 * ~~~
 *
 * @param img_type - AS_MOCK_LIB, AS_MOCK_EXEC
 * @param img_name - library to create (e.g. "libexample" will generate "libexample.so")
 * @param _extra_compiler_args - array of strings (may be NULL)
 * @return
 */
char *mock_image(int img_type, const char *img_name, char **_extra_compiler_args, int *exit_code) {
    char libsuffix[10] = {0,};
    char code[255] = {0,};
    char code_filename[FILENAME_MAX] = {0,};
    char img_filename[FILENAME_MAX] = {0,};
    char cmd[PATH_MAX * (FILENAME_MAX * 2)] = {0,};
    char *extra_compiler_args = NULL;
    Process *proc = NULL;

    if (img_name == NULL) {
        return NULL;
    }

    if (_extra_compiler_args != NULL) {
        extra_compiler_args = join(_extra_compiler_args, " ");
    }

    strcpy(libsuffix, SPM_SHLIB_EXTENSION);
    sprintf(code_filename, "%s.c", img_name);

    if (img_type == AS_MOCK_LIB) {
        sprintf(code, "int %sMockFn(void) {\n    return 0;\n}\n", img_name);
        sprintf(img_filename, "%s%s", img_name, libsuffix);
#if OS_DARWIN
        sprintf(cmd, "gcc -o %s -fPIC -dynamiclib %s -install_name @rpath/%s -Xlinker -rpath $(pwd) '%s'",
                img_filename, extra_compiler_args, img_filename, code_filename);
#elif OS_LINUX
        sprintf(cmd, "gcc -o %s -fPIC -shared %s -Wl,-rpath=$(pwd) '%s'",
            img_filename, extra_compiler_args, code_filename);
#elif OS_WINDOWS  // TODO: UNTESTED
#if defined (__MINGW32__)
     sprintf(cmd, "gcc -shared -o %s -Wl,—out-implib,%s.a -Wl,—export-all-symbols -Wl,—enable-auto-image-base '%s'",
            img_filename, img_name, extra_compiler_args, code_filename);
#elif defined (__MSC_VER)
     sprintf(cmd, "CL /LD %s", img_filename);
#endif // windows compiler ident
#endif // OS_WINDOWS
    } else if (img_type == AS_MOCK_BIN) {
        sprintf(code, "int main(int argc, char *argv[]) {\n    return 0;\n}\n");
        sprintf(img_filename, "%s", img_name);
#if OS_DARWIN
        sprintf(cmd, "gcc -o %s %s -Xlinker -rpath $(pwd) '%s'",
                img_filename, extra_compiler_args, code_filename);
#elif OS_LINUX
        sprintf(cmd, "gcc -o %s %s -Wl,-rpath=$(pwd) '%s'",
            img_filename, extra_compiler_args, code_filename);
#elif OS_WINDOWS  // TODO: UNTESTED
#if defined (__MINGW32__)
     sprintf(cmd, "gcc -o %s %s '%s'",
            img_filename, extra_compiler_args, code_filename);
#elif defined (__MSC_VER)
     sprintf(cmd, "CL /Fe\"%s\" %s", img_name, img_filename);
#endif // windows compiler ident
#endif // OS_WINDOWS
    } else {
        fprintf(stderr, "Unknown image type: %d\n", img_type);
        return NULL;
    }

    // Write sourcecode to file
    if (mock(code_filename, code, sizeof(char), strlen(code)) == 0) {
        return NULL;
    }


    if (extra_compiler_args != NULL) {
        free(extra_compiler_args);
        extra_compiler_args = NULL;
    }

    shell(&proc, SHELL_OUTPUT, cmd);
    if (proc == NULL) {
        return NULL;
    }

    if (proc->output != NULL) {
        strip(proc->output);
        if (isempty(proc->output) == 0) {
            printf("%s\n", proc->output);
        }
    }
    if (exit_code) {
        *exit_code = proc->returncode;
    }

    return realpath(img_filename, NULL);
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
