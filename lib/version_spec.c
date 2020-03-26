/**
 * @file version_spec.c
 */
#include "spm.h"

/**
 *
 * @param str
 * @return
 */
char *version_suffix_get_alpha(char *str) {
    size_t i;
    size_t len = strlen(str);
    for (i = 0; i < len; i++) {
        // return pointer to the first alphabetic character we find
        if (isalpha(str[i])) {
            return &str[i];
        }
    }
    return NULL;
}

/**
 *
 * @param str
 * @return
 */
char *version_suffix_get_modifier(char *str) {
    size_t i;
    char *modifiers[] = {
            "rc",
            "pre",
            "dev",
            "post",
            NULL,
    };
    for (i = 0; i < strlen(str); i++) {
        for (int m = 0; modifiers[m] != NULL; m++) {
            if (strncasecmp(&str[i], modifiers[m], strlen(modifiers[m])) == 0) {
                return &str[i];
            }
        }
    }
    return NULL;
}

/**
 *
 * @param str
 * @return
 */
int64_t version_suffix_modifier_calc(char *str) {
    int64_t result = 0;
    char *tmp_s = str;

    if (strncasecmp(str, "rc", 2) == 0) {
        // do rc
        tmp_s += strlen("rc");
        if (isdigit(*tmp_s)) {
            result -= atoi(tmp_s);
        }
        else {
            result -= 1;
        }
    }
    else if (strncasecmp(str, "pre", 3) == 0) {
        // do pre
        tmp_s += strlen("pre");
        if (isdigit(*tmp_s)) {
            result -= atoi(tmp_s);
        }
        else {
            result -= 1;
        }
    }
    else if (strncasecmp(str, "dev", 3) == 0) {
        // do dev
        tmp_s += strlen("dev");
        if (isdigit(*tmp_s)) {
            result -= atoi(tmp_s);
        }
        else {
            result -= 1;
        }
    }
    else if (strncasecmp(str, "post", 4) == 0) {
        // do post
        tmp_s += strlen("post");
        if (isdigit(*tmp_s)) {
            result += atoi(tmp_s);
        }
        else {
            result += 1;
        }
    }

    return result;
}

/**
 *
 * @param str
 * @return
 */
int version_suffix_alpha_calc(char *str) {
    int x = 0;
    char chs[255];
    char *ch = chs;
    memset(chs, '\0', sizeof(chs));
    strncpy(chs, str, strlen(str));

    // Handle cases where the two suffixes are not delimited by anything
    // Start scanning one character ahead of the alphabetic suffix and terminate the string
    // when/if we reach another alphabetic character (presumably a version modifer)
    for (int i = 1; chs[i] != '\0'; i++) {
        if (isalpha(chs[i])) {
            chs[i] = '\0';
        }
    }

    // Convert character to hex-ish
    x =  (*ch - 'a') + 0xa;

    // Ensure the string ends with a digit
    if (strlen(str) == 1) {
        strcat(ch, "0");
    }

    // Convert trailing numerical value to an integer
    while (*ch != '\0') {
        if (!isdigit(*ch)) {
            ch++;
            continue;
        }
        x += atoi(ch);
        break;
    }

    return x;
}

/**
 *
 * @param version_str
 * @return
 */
int64_t version_from(const char *version_str) {
    const char *delim = ".";
    int64_t result = 0;
    if (version_str == NULL) {
        return 0;
    }

    int seen_alpha = 0;     // Does the tail contain a single character, but not a modifier?
    int seen_modifier = 0;  // Does the tail contain "rc", "dev", "pre", and so forth?
    char head[255];         // digits of the string
    char tail[255];         // alphabetic characters of the string
    char *suffix_alpha = NULL;      // pointer to location of the first character after the version
    char *suffix_modifier = NULL;   // pointer to location of the modifier after the version
    char *x = NULL;         // pointer to each string delimited by "."
    char *vstr = strdup(version_str);
    if (!vstr) {
        perror("Version string copy");
        return -1;
    }

    memset(head, '\0', sizeof(head));
    memset(tail, '\0', sizeof(tail));

    // Split the version into parts
    while ((x = strsep(&vstr, delim)) != NULL) {
        int64_t tmp = 0;

        // populate the head (numeric characters)
        strncpy(head, x, strlen(x));
        for (size_t i = 0; i < strlen(head); i++) {
            if (isalpha(head[i])) {
                // populate the tail (alphabetic characters)
                strncpy(tail, &head[i], strlen(&head[i]));
                head[i] = '\0';
                break;
            }
        }

        // Detect alphabetic suffix
        if (!seen_alpha) {
            if ((suffix_alpha = version_suffix_get_alpha(x)) != NULL) {
                seen_alpha = 1;
            }
        }

        // Detect modifier suffix
        if (!seen_modifier) {
            if ((suffix_modifier = version_suffix_get_modifier(x)) != NULL) {
                seen_modifier = 1;
            }
        }

        // Stop processing if the head starts with something other than numbers
        if (!isdigit(head[0])) {
            break;
        }

        // Convert the head to an integer
        tmp = atoi(head);
        // Update result. Each portion of the numeric version is its own byte
        // Version PARTS are limited to 255
        result = result << 8 | tmp;
    }

    if (suffix_alpha != NULL) {
        // Convert the alphabetic suffix to an integer
        int64_t sac = version_suffix_alpha_calc(suffix_alpha);
        result += sac;
    }

    if (suffix_modifier != NULL) {
        // Convert the modifier string to an integer
        int64_t smc = version_suffix_modifier_calc(suffix_modifier);
        if (smc < 0) {
            result -= ~smc + 1;
        }
        else {
            result += smc;
        }
    }

    free(vstr);
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

    memset(op, '\0', NAME_MAX);
    memset(name, '\0', NAME_MAX);
    memset(version, '\0', NAME_MAX);

    // Parse name
    //for (size_t i = 0; isalnum(_strspec[i]) || _strspec[i] == '_' || _strspec[i] == '-'; i++) {
    //    name[i] = _strspec[i];
    //}
    get_name(&name, strspec);
    pos = get_operators(&op, strspec);

    ManifestPackage **m = NULL;
    // No operators found
    if (pos == NULL) {
        m = find_by_spec(manifest, name, ">=", NULL);
    }

    // When `m` is still NULL after applying the default operator
    if (m == NULL) {
        for (size_t i = 0; *(pos + i) != '\0'; i++) {
            version[i] = *(pos + i);
        }
        m = find_by_spec(manifest, name, op, version);
    }

    // When `m` has been populated by either test above, return a COPY of the manifest
    if (m != NULL) {
        ManifestPackage *selected = NULL;
        for (size_t i = 0; m[i] != NULL; i++) {
            selected = m[i];
        }

        /* TODO: What on earth was I trying to do here? Why am I freeing the manifest (pointers)
        ManifestPackage *result = manifest_package_copy(selected);
        for (size_t i = 0; m[i] != NULL; i++) {
            manifest_package_free(m[i]);
        }
        free(m);
         */
        free(strspec);
        return selected;
    }

    // Obviously it didn't work out
    free(strspec);
    return NULL;
}
