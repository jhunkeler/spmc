#include "spm.h"
#include "framework.h"

const char *testFmt = "case: '%c': returned %d, expected %d\n";
struct TestCase testCase[] = {
        {.caseValue.character = '~', .truthValue.signed_integer = 1},
        {.caseValue.character = '!', .truthValue.signed_integer = 1},
        {.caseValue.character = '=', .truthValue.signed_integer = 1},
        {.caseValue.character = '<', .truthValue.signed_integer = 1},
        {.caseValue.character = '>', .truthValue.signed_integer = 1},
        {.caseValue.character = 'u', .truthValue.signed_integer = 0},
        {.caseValue.character = 'd', .truthValue.signed_integer = 0},
        {.caseValue.character = 'l', .truthValue.signed_integer = 0},
        {.caseValue.character = 'r', .truthValue.signed_integer = 0},
        {.caseValue.character = 'b', .truthValue.signed_integer = 0},
        {.caseValue.character = 'a', .truthValue.signed_integer = 0},
        {.caseValue.character = '\n', .truthValue.signed_integer = 0},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        int result = isrelational(testCase[i].caseValue.character);
        myassert(result == testCase[i].truthValue.signed_integer, testFmt, testCase[i].caseValue.character, result, testCase[i].truthValue.signed_integer);
    }
    return 0;
}