#include "spm.h"
#include "framework.h"

const char *testFmt = "translated error code '%d': returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
#if defined(__APPLE__) && defined(__MACH__)
        {.caseValue.signed_integer = 0, .truthValue.sptr = "Undefined error: 0 (winning)", .arg[0].signed_integer = 0, .arg[1].sptr = "winning"},
        {.caseValue.signed_integer = -1, .truthValue.sptr = "Unknown error: -1 (not winning)", .arg[0].signed_integer = 0, .arg[1].sptr = "not winning"},
#elif defined(__linux) || defined(__linux__)
        {.caseValue.signed_integer = 0, .truthValue.sptr = "Success (winning)", .arg[0].signed_integer = 0, .arg[1].sptr = "winning"},
        {.caseValue.signed_integer = -1, .truthValue.sptr = "Unknown error -1 (not winning)", .arg[0].signed_integer = 0, .arg[1].sptr = "not winning"},
#endif
        {.caseValue.signed_integer = SPM_ERR_ROOT_NO_RECORD, .truthValue.sptr = "No root record (/some/path)", .arg[0].signed_integer = 0, .arg[1].sptr = "/some/path"},
        {.caseValue.signed_integer = SPM_ERR_ROOT_UNSAFE, .truthValue.sptr = "Dangerous root path (was /)", .arg[0].signed_integer = 0, .arg[1].sptr = "was /"},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        // Mock global errno value to the value stored in the test case
        errno = testCase[i].arg[0].signed_integer;
        // Mock spmerrno value
        spmerrno = testCase[i].caseValue.signed_integer;
        spmerrno_cause(testCase[i].arg[1].sptr);

        // Get SPM error (or system error)
        char *estr = spm_strerror(testCase[i].caseValue.signed_integer);

        // Assert error string matches error produced
        myassert(strcmp(estr, testCase[i].truthValue.sptr) == 0, testFmt, testCase[i].caseValue.signed_integer, estr, testCase[i].truthValue.sptr);
    }
    return 0;
}
