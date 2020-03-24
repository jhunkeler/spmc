#include "spm.h"
#include "framework.h"

const char *testFmt = "case '%s': returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.caseValue.str = "    four spaces", .truthValue.sptr = "four spaces"},
        {.caseValue.str = "\tone tab", .truthValue.sptr = "one tab"},
        {.caseValue.str = "\na new line", .truthValue.sptr = "a new line"},
        {.caseValue.sptr = NULL, .truthValue.sptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *caseValue = NULL;
        char *orig = NULL;
        char *result = NULL;

        if (testCase[i].caseValue.sptr != NULL) {
            caseValue = strdup(testCase[i].caseValue.str);
            orig = strdup(caseValue);
            result = lstrip(caseValue);
        }

        if (testCase[i].truthValue.sptr == NULL && result == NULL) {
            continue;
        } else if (testCase[i].truthValue.sptr != NULL && result == NULL) {
            fprintf(stderr, testFmt, orig, result, testCase[i].truthValue.sptr);
            return 1;
        }
        myassert(strcmp(result, testCase[i].truthValue.sptr) == 0, testFmt, orig, result, testCase[i].truthValue.sptr);
        free(caseValue);
        free(orig);
    }
    return 0;
}