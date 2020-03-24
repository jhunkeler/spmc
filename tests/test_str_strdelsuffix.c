#include "spm.h"
#include "framework.h"

const char *testFmt = "could not remove '%s' from '%s', expecting '%s'\n";
struct TestCase testCase[] = {
        {.inputValue.sptr = "gumball", .caseValue.str = "abracadabra brisket gumball", .truthValue.sptr = "abracadabra brisket"},
        {.inputValue.sptr = "y", .caseValue.str = "balloons are falling from the sky", .truthValue.sptr = "balloons are falling from the sk"},
        {.inputValue.sptr = "ickly", .caseValue.str = "mangled mangoes oxidize quickly", .truthValue.sptr = "mangled mangoes oxidize qu"},
        {.inputValue.sptr = "B", .caseValue.str = "bBbB", .truthValue.sptr = "bBb"},
        {.inputValue.sptr = NULL, .caseValue.str = "bBbB", .truthValue.sptr = "bBbB"},
        {.inputValue.sptr = "test", .caseValue.str = "\0", .truthValue.sptr = ""},
        {.inputValue.sptr = NULL, .caseValue.str = "\0", .truthValue.sptr = ""},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *orig = strdup(testCase[i].caseValue.str);
        strdelsuffix(testCase[i].caseValue.str, testCase[i].inputValue.sptr);
        myassert(strcmp(testCase[i].caseValue.str, testCase[i].truthValue.sptr) == 0, testFmt, testCase[i].inputValue.sptr, orig, testCase[i].truthValue.sptr);
        free(orig);
    }
    return 0;
}