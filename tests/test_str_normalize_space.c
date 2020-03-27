#include "spm.h"
#include "framework.h"

const char *testFmt = "case: '%s': returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.caseValue.sptr = "no extra whitespace in the string", .truthValue.sptr = "no extra whitespace in the string"},
        {.caseValue.sptr = "two  extra  spaces  in  the  string", .truthValue.sptr = "two extra spaces in the string"},
        {.caseValue.sptr = "three   extra   spaces   in   the   string", .truthValue.sptr = "three extra spaces in the string"},
        {.caseValue.sptr = "    leading    whitespace", .truthValue.sptr = "leading whitespace"},
        {.caseValue.sptr = "trailing     whitespace    ", .truthValue.sptr = "trailing whitespace"},
        {.caseValue.sptr = "    leading    and    trailing     whitespace    ", .truthValue.sptr = "leading and trailing whitespace"},
        {.caseValue.sptr = " varying  degrees           of    whitespace        everywhere ", .truthValue.sptr = "varying degrees of whitespace everywhere"},
        {.caseValue.sptr = "nowhitespace", .truthValue.sptr = "nowhitespace"},
        {.caseValue.sptr = NULL, .truthValue.sptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        if (testCase[i].caseValue.sptr == NULL && testCase[i].truthValue.sptr == NULL) {
            // null input is null output
            continue;
        }
        char *result = strdup(testCase[i].caseValue.sptr);
        normalize_space(result);
        myassert(strcmp(result, testCase[i].truthValue.sptr) == 0, testFmt, testCase[i].caseValue.sptr, result, testCase[i].truthValue.sptr);
    }
    return 0;
}