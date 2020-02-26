#include "spm.h"

static int verify_filelist(size_t lineno, char **a) {
    if (lineno == 0) {
        if (strncmp((*a), SPM_PACKAGE_HEADER_FILELIST, strlen(SPM_PACKAGE_HEADER_FILELIST)) != 0) {
            fprintf(stderr, "invalid or missing header: line %zu: %s (expected: '%s')\n",
                    lineno, (*a), SPM_PACKAGE_HEADER_FILELIST);
            return 1;
        }
    }
    return -1;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
static int verify_depends(size_t lineno, char **a) {
    return -1;
}

static int verify_descriptor(size_t lineno, char **a) {
    if (lineno == 0) {
        if (strncmp((*a), SPM_PACKAGE_HEADER_DESCRIPTOR, strlen(SPM_PACKAGE_HEADER_DESCRIPTOR)) != 0) {
            fprintf(stderr, "invalid or missing header: line %zu: %s (expected: '%s')\n",
                    lineno, (*a), SPM_PACKAGE_HEADER_DESCRIPTOR);
            return 1;
        }
    }
    return -1;
}

static int verify_prefix(size_t lineno, char **a) {
    size_t parity = lineno % 2;
    if (parity == 0 && *(*a) == '#') {
        return 0;
    }
    else {
        return 1;
    }
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
static int verify_no_op(size_t lineno, char **a) {
    return -1;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
static int reader_metadata(size_t lineno, char **line) {
    (*line) = strip((*line));
    if (isempty((*line))) {
        return 1; // indicate "continue"
    }
    return 0; // indicate "ok"
}

/**
 * Detect the type of metadata based on file name and execute the appropriate function against each line
 * in the file. Verification can be disabled by passing a non-zero value as the second argument.
 * @param _filename
 * @param no_verify SPM_METADATA_VERIFY or SPM_METADATA_NO_VERIFY
 * @return array of strings (line endings removed)
 */
char **metadata_read(const char *_filename, int no_verify) {
    char *filename = strdup(_filename);
    char **data = NULL;
    char **result = NULL;
    size_t start = 0;
    ReaderFn *func_verify;

    // Guard
    if (file_is_metadata(filename) == 0) {
        free(filename);
        return NULL;
    }

    // Setup function pointer and data starting offsets (if any)
    if (strcmp(basename(filename), SPM_META_FILELIST) == 0) {
        func_verify = verify_filelist;
        start = 1;
    } else if (strcmp(basename(filename), SPM_META_DESCRIPTOR) == 0) {
        func_verify = verify_descriptor;
        start = 1;
    } else if (strcmp(basename(filename), SPM_META_DEPENDS) == 0) {
        func_verify = verify_depends;
    } else if (strcmp(basename(filename), SPM_META_PREFIX_BIN) == 0) {
        func_verify = verify_prefix;
    } else if (strcmp(basename(filename), SPM_META_PREFIX_TEXT) == 0) {
        func_verify = verify_prefix;
    } else {
        func_verify = verify_no_op;
    }

    // Get file contents
    data = file_readlines(filename, 0, 0, reader_metadata);

    // Strip newlines and whitespace
    for (size_t i = 0; data[i] != NULL; i++) {
        data[i] = strip(data[i]);
    }

    // Perform verification
    if (no_verify == 0) {
        for (size_t i = 0; data[i] != NULL; i++) {
            int status = func_verify(i, &data[i]);
            if (status > 0) {
                fprintf(stderr, "%s: file verification failed\n", filename);
                return NULL;
            } else if (status < 0) {
                // NOT AN ERROR
                // a negative value indicates the verification function has processed enough information
                // so we can gracefully
                break;
            }
        }
    }

    // If there was a header, duplicate the array from the start of the data
    // Otherwise return the array
    if (start > 0) {
        result = strdup_array(&data[start]);
        for (size_t i = 0; data[i] != NULL; i++) {
            free(data[i]);
        }
        free(data);
    } else {
        result = data;
    }

    return result;
}
