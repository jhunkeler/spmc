#ifndef SPM_ERROR_HANDLER_H
#define SPM_ERROR_HANDLER_H

#define _SPM_ERR_BASE 0x8000                    // SPM errors begin at 32768
#define _SPM_ERR_MASK 0x7FFF                    // Support up to 32768 error strings (zero index)
#define _SPM_ERR(X) _SPM_ERR_BASE + X           // Create an error code
#define SPM_ERR_CONFIRM(X) (X >= 0x8000)        // Is X a SPM error code? (no=0, yes=!0)
#define SPM_ERR_INDEX(X) (_SPM_ERR_MASK & X)    // get index of error string

#define SPM_ERR_SUCCESS             _SPM_ERR(0)     // no error
#define SPM_ERR_ROOT_NO_RECORD      _SPM_ERR(1)     // "root" has no root record
#define SPM_ERR_ROOT_UNSAFE         _SPM_ERR(2)     // "root" at root, "/"
#define SPM_ERR_PKG_NOT_FOUND       _SPM_ERR(3)     // package not found
#define SPM_ERR_PKG_INVALID         _SPM_ERR(4)     // invalid package (wrong structure, missing data, etc)
#define SPM_ERR_PKG_CHECKSUM        _SPM_ERR(5)     // bad checksum
#define SPM_ERR_PKG_FETCH           _SPM_ERR(6)     // failed to download package

extern int spmerrno;

static const char *SPM_ERR_STRING[] = {
        "Success",
        "No root record",
        "Dangerous root path",
        "Package not found",
        "Invalid package",
        "Bad package checksum",
        "Failed to fetch package",
        NULL,
};

char *spm_strerror(int code);
void spm_perror(const char *msg);

#endif //SPM_ERROR_HANDLER_H
