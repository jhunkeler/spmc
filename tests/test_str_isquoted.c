#include "spm.h"
#include "framework.h"

const char *testFmt = "case: '%s': returned %d, expected %d\n";
struct TestCase testCase[] = {
        {.caseValue.sptr = "not quoted", .truthValue.signed_integer = 0},
        {.caseValue.sptr = "\"double quoted\"", .truthValue.signed_integer = 1},
        {.caseValue.sptr = "\'single quoted\'", .truthValue.signed_integer = 1},
        {.caseValue.sptr = "\"no closing quote", .truthValue.signed_integer = 0},
        {.caseValue.sptr = "no opening quote\"", .truthValue.signed_integer = 0},
        {.caseValue.sptr = NULL, .truthValue.signed_integer = -1},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        int result = isquoted(testCase[i].caseValue.sptr);
        myassert(result == testCase[i].truthValue.signed_integer, testFmt, testCase[i].caseValue.sptr, result, testCase[i].truthValue.signed_integer);
    }
    return 0;
}