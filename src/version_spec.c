#include "spm.h"

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

int version_suffix_alpha_calc(char *str) {
    if (version_suffix_get_modifier(str) != NULL) {
        return 0;
    }
    int x = 0;
    char chs[255];
    char *ch = chs;
    memset(chs, '\0', sizeof(chs));
    strncpy(chs, str, strlen(str));

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

int64_t version_from(const char *version_str) {
    const char *delim = ".";
    int64_t result = 0;

    int seen_alpha = 0;     // Does the tail contain a single character, but not a modifier?
    int seen_modifier = 0;  // Does the tail contain "rc", "dev", "pre", and so forth?
    char head[255];         // digits of the string
    char tail[255];         // alphabetic characters of the string
    char *suffix_alpha = NULL;      // pointer to location of the first character after the version
    char *suffix_modifier = NULL;   // pointer to location of the modifier after the version
    char *x = NULL;         // pointer to each string delimited by "."
    char *vstr = calloc(strlen(version_str) + 1, sizeof(char));
    if (!vstr) {
        perror("Version string copy");
        return -1;
    }

    memset(head, '\0', sizeof(head));
    memset(tail, '\0', sizeof(tail));
    strncpy(vstr, version_str, strlen(version_str));

    // Split the version into parts
    while ((x = strsep(&vstr, delim)) != NULL) {
        int64_t tmp = 0;

        // populate the head (numeric characters)
        strncpy(head, x, strlen(x));
        for (int i = 0; i < strlen(head); i++) {
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
