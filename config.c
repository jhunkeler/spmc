/**
 * @file config.c
 */
#include "spm.h"

/// Remove leading whitespace from a string
/// \param sptr pointer to string
/// \return pointer to first non-whitespace character in string
char *lstrip(char *sptr) {
    while (isblank(*sptr)) {
        sptr++;
    }
    return sptr;
}

/// Remove trailing whitespace from a string
/// \param sptr pointer to string
/// \return truncated string
char *strip(char *sptr) {
    char *tmp = sptr + strlen(sptr) - 1;
    while (isblank(*tmp) || *tmp == '\n') {
        *tmp = '\0';
        tmp--;
    }
    return sptr;
}

/// Determine if a string is empty
/// \param sptr pointer to string
/// \return 0=not empty, 1=empty
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

/// Determine if a string is encapsulated by quotes
/// \param sptr pointer to string
/// \return 0=not quoted, 1=quoted
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

ConfigItem **config_read(const char *filename) {
    const char sep = '=';
    char line[CONFIG_BUFFER_SIZE];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        // errno will be set, so die, and let the caller handle it
        return NULL;
    }
    int record_initial = 2;
    ConfigItem **config = (ConfigItem **) calloc(record_initial, sizeof(ConfigItem *));
    int record = 0;


    while (fgets(line, sizeof(line), fp) != NULL) {
        char *lptr = line;
        // Remove leading space and newlines
        lptr = lstrip(lptr);
        // Remove trailing space and newlines
        lptr = strip(lptr);

        // Skip empty lines
        if (isempty(lptr)) {
            continue;
        }
        // Skip comment-only lines
        if (*lptr == '#' || *lptr == ';') {
            continue;
        }

        // Get a pointer to the key pair separator
        char *sep_pos = strchr(lptr, sep);
        if (!sep_pos) {
            printf("invalid entry on line %d: missing '%c': '%s'\n", record, sep, lptr);
            continue;
        }

        // These values are approximations.  The real length(s) are recorded using strlen below.
        // At most we'll lose a few heap bytes to whitespace, but it's better than allocating PATH_MAX or BUFSIZ
        // for a measly ten byte string.
        size_t key_length = strcspn(lptr, &sep);
        size_t value_length = strlen(sep_pos);

        // Allocate a ConfigItem record
        config[record] = (ConfigItem *)calloc(1, sizeof(ConfigItem));
        config[record]->key = (char *)calloc(key_length + 1, sizeof(char));
        config[record]->value = (char *)calloc(value_length + 1, sizeof(char));

        // Shortcut our array at this point. Things get pretty ugly otherwise.
        char *key = config[record]->key;
        char *value = config[record]->value;

        // Copy the array pointers (used to populate config->key/value_length
        char *key_orig = key;
        char *value_orig = value;

        // Populate the key and remove any trailing space
        while (lptr != sep_pos) {
            *key++ = *lptr++;
        }
        key = strip(key);

        // We're at the separator now, so skip over it
        lptr++;
        // and remove any leading space
        lptr = lstrip(lptr);

        // Determine whether the string is surrounded by quotes, if so, get rid of them
        if (isquoted(lptr)) {
            // Move pointer beyond quote
            lptr = strpbrk(lptr, "'\"") + 1;
            // Terminate on closing quote
            char *tmp = strpbrk(lptr, "'\"");
            *tmp = '\0';
        }

        // Populate the value, and ignore any inline comments
        while (*lptr) {
            if (*lptr == '#' || *lptr == ';') {
                // strip trailing whitespace where the comment is and stop processing
                value = strip(value);
                break;
            }
            *value++ = *lptr++;
        }

        // Populate length data
        config[record]->key_length = strlen(key_orig);
        config[record]->value_length = strlen(value_orig);

        // increment record count
        record++;
        // Expand config by another record
        config = (ConfigItem **)reallocarray(config, record + record_initial + 1, sizeof(ConfigItem *));
    }
    return config;
}

void config_free(ConfigItem **item) {
    for (int i = 0; item[i] != NULL; i++) {
        free(item[i]);
    }
    free(item);
}

/// If the configuration contains `key` return a pointer to that record
/// \param item pointer to array of config records
/// \param key search for key in config records
/// \return success=pointer to record, failure=NULL
ConfigItem *config_get(ConfigItem **item, const char *key) {
    if (!item) {
        return NULL;
    }
    for (int i = 0; item[i] != NULL; i++) {
        if (!strcmp(item[i]->key, key)) {
            return item[i];
        }
    }
    return NULL;
}

void config_test(void) {
    ConfigItem **config = config_read("program.conf");
    printf("Data Parsed:\n");
    for (int i = 0; config[i] != NULL; i++) {
        printf("key: '%s', value: '%s'\n", config[i]->key, config[i]->value);
    }

    printf("Testing config_get():\n");
    ConfigItem *cptr = NULL;
    if ((cptr = config_get(config, "integer_value"))) {
        printf("%s = %d\n", cptr->key, atoi(cptr->value));
    }
    if ((cptr = config_get(config, "float_value"))) {
        printf("%s = %.3f\n", cptr->key, atof(cptr->value));
    }
    if ((cptr = config_get(config, "string_value"))) {
        printf("%s = %s\n", cptr->key, cptr->value);
    }
    config_free(config);
}
