/**
 * @file config.c
 */
#include "spm.h"

/**
 * Parse a basic configuration file
 *
 * NOTE: All values are stored as strings. You need to convert non-string values yourself.
 *
 * Example:
 * ~~~{.c}
 * ConfigItem **items = config_read("example.cfg");
 * if (!items) {
 *     // handle error
 * }
 * config_free(items);
 * ~~~
 *
 *
 * @param filename
 * @return success=`ConfigItem` array, failure=NULL
 */
ConfigItem **config_read(const char *filename) {
    const char sep = '=';
    size_t record = 0;
    char *line = NULL;
    FILE *fp = NULL;

    if (SPM_GLOBAL.verbose) {
        printf("Reading configuration file: %s\n", filename);
    }

    fp = fopen(filename, "r");
    if (!fp) {
        // errno will be set, so die, and let the caller handle it
        return NULL;
    }

    ConfigItem **config = (ConfigItem **) calloc(record + 1, sizeof(ConfigItem *));
    if (!config) {
        perror("ConfigItem");
        fprintf(SYSERROR);
        fclose(fp);
        return NULL;
    }

    line = (char *)calloc(CONFIG_BUFFER_SIZE, sizeof(char));
    if (!line) {
        perror("config line buffer");
        fprintf(SYSERROR);
        return NULL;
    }

    while (fgets(line, CONFIG_BUFFER_SIZE - 1, fp) != NULL) {
        char *lptr = line;
        // Remove trailing space and newlines
        lptr = strip(lptr);

        // Remove leading space and newlines
        lptr = lstrip(lptr);

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
            printf("invalid entry on line %zu: missing '%c': '%s'\n", record, sep, lptr);
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

        if (!config[record] || !config[record]->key || !config[record]->value) {
            perror("ConfigItem record");
            fprintf(SYSERROR);
            return NULL;
        }

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
        key = strip(key_orig);

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

        // Destroy contents of line buffer
        memset(line, '\0', CONFIG_BUFFER_SIZE);

        if (SPM_GLOBAL.verbose) {
            printf("CONFIG RECORD=%zu, PTR='%p', KEY='%s', VALUE='%s'\n",
                    record, config[record], config[record]->key, config[record]->value);
        }

        // increment record count
        record++;

        // Expand config by another record
        config = (ConfigItem **)reallocarray(config, record + 1, sizeof(ConfigItem *));
        if (!config) {
            perror("ConfigItem array");
            fprintf(SYSERROR);
            free(line);
            return NULL;
        }

        config[record] = NULL;
    }
    free(line);
    return config;
}

/**
 * Free memory allocated by `config_read`
 * @param item `ConfigItem` array
 */
void config_free(ConfigItem **item) {
    for (size_t i = 0; item[i] != NULL; i++) {
        free(item[i]->key);
        free(item[i]->value);
        free(item[i]);
    }
    free(item);
}

/**
 * If the `ConfigItem` array contains `key`, return a pointer to that record
 *
 * Example:
 * ~~~{.c}
 * char *nptr = NULL;
 * ConfigItem *item = config_get(items, "a_number");
 * if (!item) {
 *     // handle error
 * }
 * int the_number = strtol(item->value, &nptr, 10);
 * printf("%s = %d\n", item->key, the_number);
 * ~~~
 *
 * @param item pointer to array of config records
 * @param key search for key in config records
 * @return success=pointer to record, failure=NULL
 */
ConfigItem *config_get(ConfigItem **item, const char *key) {
    if (!item) {
        return NULL;
    }
    for (size_t i = 0; item[i] != NULL; i++) {
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
