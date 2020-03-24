#include "spm.h"
#include "framework.h"

const char *testFmt = "case '%s': returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.inputValue.sptr = " ",
         .caseValue.str = "after the world stops spinning\0",
         .truthValue.sptr = "aftertheworldstopsspinning"},

        {.inputValue.sptr = "a",
         .caseValue.str = "apples exploded all over the place",
         .truthValue.sptr = "pples exploded ll over the plce"},

        {.inputValue.sptr = "123456789",
         .caseValue.str = "a1a2a3a4a z!z@z#z$z a5a6a7a8a9",
         .truthValue.sptr = "aaaaa z!z@z#z$z aaaaa"},

        {.inputValue.sptr = "a",
         .caseValue.str = "aAaA",
         .truthValue.sptr = "AA"},

        {.inputValue.sptr = "A",
         .caseValue.str = "aAaA",
         .truthValue.sptr = "aa"},

        {.inputValue.sptr = "brisket",
         .caseValue.str = "abracadabra brisket gumball",
         .truthValue.sptr = "aacadaa  gumall"},

        {.inputValue.sptr = "b",
         .caseValue.str = "balloons are falling from the sky",
         .truthValue.sptr = "alloons are falling from the sky"},

        {.inputValue.sptr = "ly",
         .caseValue.str = "mangled mangoes oxidize quickly",
         .truthValue.sptr = "manged mangoes oxidize quick"},

        {.inputValue.sptr = NULL,
         .caseValue.str = "bBbB",
         .truthValue.sptr = "bBbB"},

        {.inputValue.sptr = "B",
         .caseValue.str = "\0",
         .truthValue.sptr = ""},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *orig = strdup(testCase[i].caseValue.str);
        strchrdel(testCase[i].caseValue.str, testCase[i].inputValue.sptr);
        myassert(strcmp(testCase[i].caseValue.str, testCase[i].truthValue.sptr) == 0,
                testFmt,
                orig,
                testCase[i].caseValue.str,
                testCase[i].truthValue.sptr);
        free(orig);
    }

    return 0;
}
