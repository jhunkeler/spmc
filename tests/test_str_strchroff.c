#include "spm.h"
#include "framework.h"

const char *testFmt = "case '%s': returned %d, expected %d\n";
struct TestCase testCase[] = {
        {.inputValue.unsigned_integer = 'a', .caseValue.sptr = "after the world stops spinning", .truthValue.signed_integer = 0},
        {.inputValue.unsigned_integer = 'p', .caseValue.sptr = "apples exploded all over the place", .truthValue.signed_integer = 1},
        {.inputValue.unsigned_integer = '2', .caseValue.sptr = "a1a2a3a4a z!z@z#z$z a5a6a7a8a9", .truthValue.signed_integer = 3},
        {.inputValue.unsigned_integer = 'a', .caseValue.sptr = "aAaA", .truthValue.signed_integer = 0},
        {.inputValue.unsigned_integer = 'A', .caseValue.sptr = "aAaA", .truthValue.signed_integer = 1},
        {.inputValue.unsigned_integer = 'g', .caseValue.sptr = "abracadabra brisket gumball", .truthValue.signed_integer = 20},
        {.inputValue.unsigned_integer = 'r', .caseValue.sptr = "balloons are falling from the sky", .truthValue.signed_integer = 10},
        {.inputValue.unsigned_integer = 'b', .caseValue.sptr = "mangled mangoes oxidize quickly", .truthValue.signed_integer = -1},
        {.inputValue.unsigned_integer = 'b', .caseValue.sptr = "bBbB", .truthValue.signed_integer = 0},
        {.inputValue.unsigned_integer = 'B', .caseValue.sptr = "bBbB", .truthValue.signed_integer = 1},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);


int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        int result = strchroff(testCase[i].caseValue.sptr, testCase[i].inputValue.unsigned_integer);
        myassert(result == testCase[i].truthValue.signed_integer,
                testFmt,
                testCase[i].caseValue.sptr,
                result,
                testCase[i].truthValue.signed_integer);
    }

    return 0;
}
