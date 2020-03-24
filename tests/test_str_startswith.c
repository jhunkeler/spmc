#include "spm.h"
#include "framework.h"

const char *testFmt = "'%s' does not start with '%s' (%d)\n";
struct TestCase testCase[] = {
        {.inputValue.sptr = "abra", .caseValue.sptr = "abracadabra brisket gumball", .truthValue.signed_integer = 1},
        {.inputValue.sptr = "ball", .caseValue.sptr = "balloons are falling from the sky", .truthValue.signed_integer = 1},
        {.inputValue.sptr = "mangle", .caseValue.sptr = "mangled mangoes oxidize quickly", .truthValue.signed_integer = 1},
        {.inputValue.sptr = "b", .caseValue.sptr = "bBbB", .truthValue.signed_integer = 1},
        {.inputValue.sptr = NULL, .caseValue.sptr = "bBbB", .truthValue.signed_integer = -1},
        {.inputValue.sptr = "test", .caseValue.sptr = NULL, .truthValue.signed_integer = -1},
        {.inputValue.sptr = NULL, .caseValue.sptr = NULL, .truthValue.signed_integer = -1},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        int result = startswith(testCase[i].caseValue.sptr, testCase[i].inputValue.sptr);
        myassert(result == testCase[i].truthValue.signed_integer, testFmt, testCase[i].inputValue.str, testCase[i].truthValue.sptr, result);
    }
    return 0;
}