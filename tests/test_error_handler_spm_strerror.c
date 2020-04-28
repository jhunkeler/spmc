#include "spm.h"
#include "framework.h"

const char *testFmt = "translated error code '%d': returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
#if OS_DARWIN
        {.caseValue.signed_int = 0, .truthValue.sptr = "Undefined error: 0", .arg[0].signed_int = 0},
        {.caseValue.signed_int = -1, .truthValue.sptr = "Unknown error: -1", .arg[0].signed_int = 0},
#elif OS_LINUX
        {.caseValue.signed_int = 0, .truthValue.sptr = "Success", .arg[0].signed_int = 0},
        {.caseValue.signed_int = -1, .truthValue.sptr = "Unknown error -1", .arg[0].signed_int = 0},
#endif
        {.caseValue.signed_int = SPM_ERR_ROOT_NO_RECORD, .truthValue.sptr = "No root record", .arg[0].signed_int = 0},
        {.caseValue.signed_int = SPM_ERR_ROOT_UNSAFE, .truthValue.sptr = "Dangerous root path", .arg[0].signed_int = 0},
        {.caseValue.signed_int = ENOENT, .truthValue.sptr = "No such file or directory", .arg[0].signed_int = ENOENT},
        {.caseValue.signed_int = EPIPE, .truthValue.sptr = "Broken pipe", .arg[0].signed_int = EPIPE},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        // Mock global errno value to the value stored in the test case
        errno = testCase[i].arg[0].signed_int;

        // Get SPM error (or system error)
        char *estr = spm_strerror(testCase[i].caseValue.signed_int);

        // Assert error string matches error produced
        myassert(strcmp(estr, testCase[i].truthValue.sptr) == 0, testFmt, testCase[i].caseValue.signed_int, estr, testCase[i].truthValue.sptr);
    }
    return 0;
}
