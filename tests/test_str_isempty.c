#include "spm.h"
#include "framework.h"

const char *testFmt = "case: '%s': returned %d, expected %d\n";
struct TestCase testCase[] = {
        {.caseValue.sptr = " not empty", .truthValue.signed_integer = 0},
        {.caseValue.sptr = "not empty", .truthValue.signed_integer = 0},
        {.caseValue.sptr = "    ", .truthValue.signed_integer = 1},
        {.caseValue.sptr = "\t", .truthValue.signed_integer = 1},
        {.caseValue.sptr = "\n", .truthValue.signed_integer = 1},
        {.caseValue.sptr = "", .truthValue.signed_integer = 1},
        {.caseValue.sptr = NULL, .truthValue.signed_integer = -1},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        int result = isempty(testCase[i].caseValue.sptr);
        myassert(result == testCase[i].truthValue.signed_integer, testFmt, testCase[i].caseValue.sptr, result, testCase[i].truthValue.sptr);
    }
    return 0;
}