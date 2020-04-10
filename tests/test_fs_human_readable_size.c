#include "spm.h"
#include "framework.h"

const char *testFmt = "returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.caseValue.unsigned_long = 1L, .truthValue.sptr = "1B"},
        {.caseValue.unsigned_long = 1L * 1024, .truthValue.sptr = "1.00K"},
        {.caseValue.unsigned_long = 1L * 1024 * 1024, .truthValue.sptr = "1.00M"},
        {.caseValue.unsigned_long = 1L * 1024 * 1024 * 1024, .truthValue.sptr = "1.00G"},
        {.caseValue.unsigned_long = 1L * 1024 * 1024 * 1024 * 1024, .truthValue.sptr = "1.00T"},
        {.caseValue.unsigned_long = 1L * 1024 * 1024 * 1024 * 1024 * 1024, .truthValue.sptr = "1.00P"},
        {.caseValue.unsigned_long = 1L * 1024 * 1024 * 1024 * 1024 * 1024 * 1024, .truthValue.sptr = "1.00E"},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *result = human_readable_size(testCase[i].caseValue.unsigned_long);
        myassert(strcmp(result, testCase[i].truthValue.sptr) == 0, testFmt, result, testCase[i].truthValue.sptr);
    }
    return 0;
}