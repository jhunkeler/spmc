#include "spm.h"
#include "framework.h"

const char *testFmt = "case: returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.inputValue.sptr = " ", .truthValue.sptr = "one two three"},
        {.inputValue.sptr = "|", .truthValue.sptr = "one|two|three"},
        {.inputValue.sptr = ",", .truthValue.sptr = "one,two,three"},
        {.inputValue.sptr = "b", .truthValue.sptr = "onebtwobthree"},
        {.inputValue.sptr = NULL, .truthValue.slptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *result = join_ex(testCase[i].inputValue.sptr, "one", "two", "three", NULL);
        if (testCase[i].truthValue.sptr == NULL && result == NULL) {
            continue;
        }
        myassert(strcmp(result, testCase[i].truthValue.sptr) == 0, testFmt, result, testCase[i].truthValue.sptr);
        free(result);
    }
    return 0;
}