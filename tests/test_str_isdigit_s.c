#include "spm.h"
#include "framework.h"

const char *testFmt = "case '%s': returned '%d', expected '%d'\n";
struct TestCase testCase[] = {
        {.caseValue.str = "1234", .truthValue.signed_int = 1},
        {.caseValue.str = "1234000000000", .truthValue.signed_int = 1},
        {.caseValue.str = "1234aa", .truthValue.signed_int = 0},
        {.caseValue.str = "z1234a", .truthValue.signed_int = 0},
        {.caseValue.str = "gabcde", .truthValue.signed_int = 0},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        int result = isdigit_s(testCase[i].caseValue.str);
        int truth = testCase[i].truthValue.signed_int;
        myassert(result == truth, testFmt, testCase[i].caseValue.str, result, truth);
    }
    return 0;
}