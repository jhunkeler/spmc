#include "spm.h"
#include "framework.h"

const char *testFmt = "case '%s': returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.inputValue.sptr = "[]", .caseValue.str = "[one two three]", .truthValue.sptr = "one two three"},
        {.inputValue.sptr = "..", .caseValue.str = ".[one].", .truthValue.sptr = "[one]"},
        {.inputValue.sptr = "\'\'", .caseValue.str = ".['one'].", .truthValue.sptr = "one"},
        {.inputValue.sptr = "@^", .caseValue.str = "abcdef@ghijkl^mnop@qrst^uvwxyz", .truthValue.sptr = "ghijkl"},
        {.inputValue.sptr = "", .caseValue.sptr = NULL, .truthValue.sptr = NULL},
        {.inputValue.sptr = NULL, .caseValue.sptr = NULL, .truthValue.sptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *result = substring_between(testCase[i].caseValue.str, testCase[i].inputValue.sptr);
        if (testCase[i].truthValue.sptr == NULL && result == NULL) {
            continue;
        } else if (testCase[i].truthValue.sptr != NULL && result == NULL) {
            fprintf(stderr, testFmt, testCase[i].caseValue.str, result, testCase[i].truthValue.sptr);
            return 1;
        }
        myassert(strcmp(result, testCase[i].truthValue.sptr) == 0, testFmt, testCase[i].caseValue.str, result, testCase[i].truthValue.sptr);
        free(result);
    }
    return 0;
}