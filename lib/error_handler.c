#include "spm.h"

int spmerrno = 0;
static char spmerrbuf[255];
static char spmerrbuf_reason[255];
const char *SPM_ERR_STRING[] = {
        "Success",
        "No root record",
        "Dangerous root path",
        "Package not found",
        "Invalid package",
        "Bad package checksum",
        "Failed to fetch package",
        "Manifest has no header",
        "Manifest has no data",
        "Parsing error",
        "Not implemented",
        "Failed to fetch data",
        NULL,
};

/**
 * Append error text to an existing spmerrno error string
 * @param reason message text
 */
void spmerrno_cause(const char *reason) {
    char *buf = spmerrbuf_reason;
    if (reason != NULL) {
        sprintf(buf, " (%s)", reason);
    }
}

/**
 * Translate a spmerrno code to an error string
 * @param code
 * @return
 */
char *spm_strerror(int code) {
    char *buf = spmerrbuf;
    int is_spm_error = SPM_ERR_CONFIRM(code);

    memset(buf, '\0', sizeof(spmerrbuf));
    if (is_spm_error == 0) {
        strcpy(buf, strerror(code));
    } else {
        strcpy(buf, SPM_ERR_STRING[SPM_ERR_INDEX(code)]);
    }

    if (strlen(spmerrbuf_reason)) {
        strcat(buf, spmerrbuf_reason);
    }
    return buf;
}

void spm_perror(const char *msg) {
    fprintf(stderr, "%s: %s\n", msg ? msg : "", spm_strerror(spmerrno));
}

void spm_die(void) {
    spm_perror("FATAL");
    exit(1);
}
