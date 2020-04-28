#include "spm.h"
#include "framework.h"

const char *testFmt = "case '%s': returned %d, expected %d\n";
struct TestCase testCase[] = {
        {.inputValue.unsigned_int = 'a', .caseValue.sptr = "after the world stops spinning", .truthValue.signed_int = 1},
        {.inputValue.unsigned_int = 'a', .caseValue.sptr = "apples exploded all over the place", .truthValue.signed_int = 3},
        {.inputValue.unsigned_int = 'a', .caseValue.sptr = "a1a2a3a4a z!z@z#z$z a5a6a7a8a9", .truthValue.signed_int = 10},
        {.inputValue.unsigned_int = 'a', .caseValue.sptr = "aAaA", .truthValue.signed_int = 2},
        {.inputValue.unsigned_int = 'A', .caseValue.sptr = "aAaA", .truthValue.signed_int = 2},
        {.inputValue.unsigned_int = 'b', .caseValue.sptr = "abracadabra brisket gumball", .truthValue.signed_int = 4},
        {.inputValue.unsigned_int = 'b', .caseValue.sptr = "balloons are falling from the sky", .truthValue.signed_int = 1},
        {.inputValue.unsigned_int = 'b', .caseValue.sptr = "mangled mangoes oxidize quickly", .truthValue.signed_int = 0},
        {.inputValue.unsigned_int = 'b', .caseValue.sptr = "bBbB", .truthValue.signed_int = 2},
        {.inputValue.unsigned_int = 'B', .caseValue.sptr = "bBbB", .truthValue.signed_int = 2},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);


int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        int result = num_chars(testCase[i].caseValue.sptr, testCase[i].inputValue.unsigned_int);
        myassert(result == testCase[i].truthValue.signed_int,
                testFmt,
                testCase[i].caseValue.sptr,
                result,
                testCase[i].truthValue.signed_int);
    }

    return 0;
}
