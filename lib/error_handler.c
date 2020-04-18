#include "spm.h"

int spmerrno = 0;
static char spmerrbuf[255];

/**
 *
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
    return buf;
}

void spm_perror(const char *msg) {
    fprintf(stderr, "%s: %s\n", msg ? msg : "", spm_strerror(spmerrno));
}

