#include "spm.h"
#include "framework.h"

const char *testFmt = "case: '%c': returned %d, expected %d\n";
struct TestCase testCase[] = {
        {.caseValue.signed_char = '~', .truthValue.signed_int = 1},
        {.caseValue.signed_char = '!', .truthValue.signed_int = 1},
        {.caseValue.signed_char = '=', .truthValue.signed_int = 1},
        {.caseValue.signed_char = '<', .truthValue.signed_int = 1},
        {.caseValue.signed_char = '>', .truthValue.signed_int = 1},
        {.caseValue.signed_char = 'u', .truthValue.signed_int = 0},
        {.caseValue.signed_char = 'd', .truthValue.signed_int = 0},
        {.caseValue.signed_char = 'l', .truthValue.signed_int = 0},
        {.caseValue.signed_char = 'r', .truthValue.signed_int = 0},
        {.caseValue.signed_char = 'b', .truthValue.signed_int = 0},
        {.caseValue.signed_char = 'a', .truthValue.signed_int = 0},
        {.caseValue.signed_char = '\n', .truthValue.signed_int = 0},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        int result = isrelational(testCase[i].caseValue.signed_char);
        myassert(result == testCase[i].truthValue.signed_int, testFmt, testCase[i].caseValue.signed_char, result, testCase[i].truthValue.signed_int);
    }
    return 0;
}