/**
 * @file version_spec.c
 */
#include "spm.h"

/**
 * Get the ASCII index of a character from '0'
 * @param c
 * @return
 */
static int version_reindex(char c) {
    int result;
    result = c - '0';
    return result;
}

/**
 * Initialize `struct Version` for use by other `version_` functions
 * @return pointer to `struct Version`
 */
struct Version *version_init() {
    struct Version *result;

    result = calloc(1, sizeof(struct Version));
    result->local = calloc(VERSION_LOCAL_MAX, sizeof(char));

    for (size_t i = 0; i < VERSION_MAX; i++) {
        result->part[i] = 0;
    }

    return result;
}

/**
 * Populate `Version` struct with information derived from input string `s`
 * @param version `struct Version`
 * @param s version string
 * @return 0=success, <0=error
 */
int version_read(struct Version **version, char *s) {
    char *str;
    char **part;
    char *local_version = NULL;
    size_t num_parts;

    if (s == NULL) {
        return -1;
    }

    str = strdup(s);
    if (str == NULL) {
        return -1;
    }

    str = tolower_s(str);

    // Store any local version data stored in the input string (unused)
    if ((local_version = strstr(str, VERSION_DELIM_LOCAL)) != NULL) {
        strncpy((*version)->local, local_version + 1, VERSION_LOCAL_MAX - 1);
        *local_version = '\0';
    }

    // Split the input string
    part = split(str, VERSION_DELIM);

    // Count records split from the input string
    for (num_parts = 0; part[num_parts] != NULL; num_parts++);

    // Last record of version->part is reserved for additional information
    uint32_t *addendum = &(*version)->part[VERSION_MAX - 1];

    // Turn those numbers into... numbers
    for (size_t i = 0; i < num_parts; i++) {
        uint32_t value = 0;

        if (isdigit_s(part[i])) {
            // Store the value as-is
            value = strtoul(part[i], NULL, VERSION_BASE);
        } else {
            char tmp[255] = {0};
            char *other = part[i];
            if (isdigit(part[i][0])) {
                for (size_t c = 0; isdigit(part[i][c]); c++) {
                    tmp[c] = part[i][c];
                    other++;
                }
                value = strtoul(tmp, NULL, VERSION_BASE);
            }
            // Reindex all alphanumeric characters from ASCII '0'. Ignore everything else.
            for (size_t c = 0; c < strlen(other); c++) {
                if (isalnum(other[c])) {
                    // Assign version addendum field
                    *addendum += version_reindex(other[c]);
                }
            }
        }
        // Assign part with integer value
        (*version)->part[i] = value;
    }

    // Convert all of the individual parts into a single integer
    for (size_t i = 0; i < VERSION_MAX; i++) {
        (*version)->asInt = (*version)->asInt << 8 | (*version)->part[i];
    }

    split_free(part);
    free(str);
    return 0;
}

/**
 * Free `struct Version` populated by `version_init`
 * @param version
 */
void version_free(struct Version *version) {
    if (version == NULL) {
        return;
    }

    free(version->local);
    free(version);
}

/**
 * Print information about a version (stdout)
 * @param version `struct Version`
 */
void version_info(struct Version *version) {
    printf("major: %d, ", version->part[0]);
    printf("minor: %d, ", version->part[1]);
    printf("patch: %d, ", version->part[2]);
    for (size_t i = 3; i < VERSION_MAX; i++) {
        printf("other[%zu]: %d, ", i, version->part[i]);
    }
    printf("local: '%s', ", version->local);
    printf("int  : %#llx (%llu)\n", version->asInt, version->asInt);
}

/**
 * Convert version string to an integer
 * @param str version string
 * @return version as integer
 */
uint64_t version_from(const char *str) {
    // Accept NULL as version "zero"
    if (str == NULL) {
        return 0;
    }

    uint64_t result = 0LL;
    char *v = strdup(str);
    struct Version *version = version_init();

    version_read(&version, v);
    result = version->asInt;

    version_free(version);
    return result;
}

/**
 *
 * @param op
 * @return
 */
int version_spec_from(const char *op) {
    int flags = VERSION_NOOP;
    size_t len = strlen(op);
    for (size_t i = 0; i < len; i++) {
        if (op[i] == '>') {
            flags |= VERSION_GT;
        }
        else if (op[i] == '<') {
            flags |= VERSION_LT;
        }
        else if (op[i] == '=' || (len > 1 && strncmp(&op[i], "==", 2) == 0)) {
            flags |= VERSION_EQ;
        }
        else if (op[i] == '!') {
            flags |= VERSION_NE;
        }
        else if (op[i] == '~') {
            flags |= VERSION_COMPAT;
        }
    }
    return flags;
}

/**
 *
 * @param a
 * @param b
 * @return
 */
static int _find_by_spec_compare(const void *a, const void *b) {
    const ManifestPackage *aa = *(const ManifestPackage**)a;
    const ManifestPackage *bb = *(const ManifestPackage**)b;
    int64_t version_a = version_from(aa->version);
    int64_t version_b = version_from(bb->version);
    return version_a > version_b;
}

/**
 *
 * @param manifest
 * @param name
 * @param op
 * @param version_str
 * @return
 */
ManifestPackage **find_by_spec(const Manifest *manifest, const char *name, const char *op, const char *version_str) {
    size_t record = 0;
    ManifestPackage **list = (ManifestPackage **) calloc(manifest->records + 1, sizeof(ManifestPackage *));
    if (!list) {
        perror("ManifestPackage array");
        fprintf(SYSERROR);
        return NULL;
    }

    for (size_t i = 0; i < manifest->records; i++) {
        if (strcmp(manifest->packages[i]->name, name) == 0) {
            int64_t version_a = version_from(manifest->packages[i]->version);
            int64_t version_b = version_from(version_str);
            int spec = version_spec_from(op);

            int res = 0;
            if (spec & VERSION_GT && spec & VERSION_EQ) {
                res = version_a >= version_b;
            }
            else if (spec & VERSION_LT && spec & VERSION_EQ) {
                res = version_a <= version_b;
            }
            else if (spec & VERSION_NE && spec & VERSION_EQ) {
                res = version_a != version_b;
            }
            else if (spec & VERSION_GT) {
                res = version_a > version_b;
            }
            else if (spec & VERSION_LT) {
                res = version_a < version_b;
            }
            else if (spec & VERSION_COMPAT) {
                // TODO
            }
            else if (spec & VERSION_EQ) {
                res = version_a == version_b;
            }

            if (res != 0) {
                list[record] = manifest_package_copy(manifest->packages[i]);
                if (!list[record]) {
                    perror("Unable to allocate memory for manifest record");
                    fprintf(SYSERROR);
                    return NULL;
                }
                record++;
            }
        }
    }
    qsort(list, record, sizeof(ManifestPackage *), _find_by_spec_compare);

    return list;
}

static void get_name(char **buf, const char *_str) {
    char *str = strdup(_str);
    int has_relational = 0;
    int is_archive = endswith(str, SPM_PACKAGE_EXTENSION);
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (isrelational(str[i]))
            has_relational = 1;
    }

    if (is_archive == 0 && !has_relational) {
        strcpy((*buf), str);
    }
    else if (has_relational) {
        size_t stop = 0;
        for (stop = 0; !isrelational(str[stop]); stop++);
        strncpy((*buf), str, stop);
        (*buf)[stop] = '\0';
    } else {
        StrList *tmp = strlist_init();
        char sep[2];
        sep[0] = SPM_PACKAGE_MEMBER_SEPARATOR;
        sep[1] = '\0';

        char **parts = split(str, sep);
        if (parts != NULL) {
            for (size_t i = 0; parts[i] != NULL; i++) {
                strlist_append(tmp, parts[i]);
            }
        }
        split_free(parts);

        if (strlist_count(tmp) > SPM_PACKAGE_MIN_DELIM) {
            strlist_set(tmp, strlist_count(tmp) - SPM_PACKAGE_MIN_DELIM, NULL);
        }
        char *result = join(tmp->data, sep);
        strcpy((*buf), result);
        free(result);
        strlist_free(tmp);
    }
    free(str);
}

static char *get_operators(char **op, const char *_strspec) {
    const char *operators = VERSION_OPERATORS;  // note: whitespace is synonymous with ">=" if no operators are present
    char *pos = NULL;
    pos = strpbrk(_strspec, operators);
    if (pos != NULL) {
        for (size_t i = 0; !isalnum(*pos) || *pos == '.'; i++) {
            (*op)[i] = *pos++;
        }
    }
    return pos;
}

ManifestPackage *find_by_strspec(const Manifest *manifest, const char *_strspec) {
    char *pos = NULL;
    char s_op[NAME_MAX];
    char s_name[NAME_MAX];
    char s_version[NAME_MAX];
    char *op = s_op;
    char *name = s_name;
    char *version = s_version;
    char *strspec = strdup(_strspec);
    ManifestPackage **m = NULL;
    ManifestPackage *selected = NULL;

    memset(op, '\0', NAME_MAX);
    memset(name, '\0', NAME_MAX);
    memset(version, '\0', NAME_MAX);

    // Parse package name
    get_name(&name, strspec);
    // Get the starting address of any operator(s) (>, <, =, etc)
    pos = get_operators(&op, strspec);

    // No operator(s) found
    if (pos == NULL) {
        // Try `name >= 0`
        m = find_by_spec(manifest, name, ">=", NULL);
    }

    // When `m` is still NULL after applying the default operator
    if (m == NULL) {
        // Parse the version string if it's there
        for (size_t i = 0; *(pos + i) != '\0'; i++) {
            version[i] = *(pos + i);
        }
        // Try `name op version`
        m = find_by_spec(manifest, name, op, version);
    }

    // Select the highest priority package
    if (m != NULL) {
        // (m[0] == default manifest, m[>0] == user-defined manifest)
        for (size_t i = 0; m[i] != NULL; i++) {
            selected = m[i];
        }
    }

    free(strspec);
    return selected;  // or NULL
}
