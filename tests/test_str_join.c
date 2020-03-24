#include "spm.h"
#include "framework.h"

const char *testFmt = "case: (array){%s}: returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.inputValue.sptr = " ", .caseValue.slptr = (char *[]){"one", "two", "three", NULL}, .truthValue.sptr = "one two three"},
        {.inputValue.sptr = "|", .caseValue.slptr = (char *[]){"one", "two", "three", NULL}, .truthValue.sptr = "one|two|three"},
        {.inputValue.sptr = ",", .caseValue.slptr = (char *[]){"one", "two", "three", NULL}, .truthValue.sptr = "one,two,three"},
        {.inputValue.sptr = "_", .caseValue.slptr = (char *[]){"one", NULL}, .truthValue.sptr = "one"},
        {.inputValue.sptr = "b", .caseValue.sptr = NULL, .truthValue.slptr = NULL},
        {.inputValue.sptr = NULL, .caseValue.sptr = NULL, .truthValue.slptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *result = join(testCase[i].caseValue.slptr, testCase[i].inputValue.sptr);
        if (testCase[i].truthValue.sptr == NULL && result == NULL) {
            continue;
        }
        myassert(strcmp(result, testCase[i].truthValue.sptr) == 0, testFmt, array_to_string(testCase[i].caseValue.slptr, ", "), result, testCase[i].truthValue.sptr);
        free(result);
    }
    return 0;
}