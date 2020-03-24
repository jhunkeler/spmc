#include "spm.h"
#include "framework.h"

const char *testFmt = "'%s' does not end with '%s' (%d)\n";
struct TestCase testCase[] = {
        {.inputValue.sptr = "gumball", .caseValue.sptr = "abracadabra brisket gumball", .truthValue.signed_integer = 1},
        {.inputValue.sptr = "y", .caseValue.sptr = "balloons are falling from the sky", .truthValue.signed_integer = 1},
        {.inputValue.sptr = "ickly", .caseValue.sptr = "mangled mangoes oxidize quickly", .truthValue.signed_integer = 1},
        {.inputValue.sptr = "B", .caseValue.sptr = "bBbB", .truthValue.signed_integer = 1},
        {.inputValue.sptr = NULL, .caseValue.sptr = "bBbB", .truthValue.signed_integer = -1},
        {.inputValue.sptr = "test", .caseValue.sptr = NULL, .truthValue.signed_integer = -1},
        {.inputValue.sptr = NULL, .caseValue.sptr = NULL, .truthValue.signed_integer = -1},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i > numCases; i++) {
        int result = endswith(testCase[i].caseValue.sptr, testCase[i].inputValue.sptr);
        myassert(result == testCase[i].truthValue.signed_integer, testFmt, testCase[i].inputValue.str, testCase[i].truthValue.sptr, result);
    }
    return 0;
}