#include "spm.h"
#include "framework.h"

const char *testFmt = "returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.caseValue.sptr = "~", .truthValue.sptr = NULL},
        {.caseValue.sptr = "/dev/winning", .truthValue.sptr = "/dev/winning"},
        {.caseValue.sptr = "", .truthValue.sptr = ""},
        {.caseValue.sptr = NULL, .truthValue.sptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
#if !defined(_WIN32)
    testCase[0].truthValue.sptr = getenv("HOME");
#else
    testcase[0].truthValue.sptr = getenv("USERPROFILE");
#endif

    for (size_t i = 0; i < numCases; i++) {
        char *result = expandpath(testCase[i].caseValue.sptr);


        if (result == NULL && testCase[i].caseValue.sptr == NULL && testCase[i].truthValue.sptr == NULL) {
            continue;
        }

        myassert(strcmp(result, testCase[i].truthValue.sptr) == 0, testFmt, result, testCase[i].truthValue.sptr);

        if (result != NULL) {
            free(result);
        }
    }
    return 0;
}