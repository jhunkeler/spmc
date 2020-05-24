#include "spm.h"
#include "framework.h"

const char *testFmt = "case '%s': returned '%d', expected '%d'\n";
struct TestCase testCase[] = {
        {.caseValue.str = "ThE", .truthValue.sptr = "the"},
        {.caseValue.str = "KIdS", .truthValue.sptr = "kids"},
        {.caseValue.str = "AREn'T", .truthValue.sptr= "aren't"},
        {.caseValue.str = "aLL", .truthValue.sptr = "all"},
        {.caseValue.str = "RiGHt", .truthValue.sptr = "right"},
        {.caseValue.str = "By", .truthValue.sptr = "by"},
        {.caseValue.str = "THe oFfSpRiNg", .truthValue.sptr = "the offspring"},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *orig = strdup(testCase[i].caseValue.str);
        char *result = tolower_s(testCase[i].caseValue.str);
        const char *truth = testCase[i].truthValue.sptr;
        myassert(strcmp(result, truth) == 0, testFmt, orig, result, truth);
        free(orig);
    }
    return 0;
}