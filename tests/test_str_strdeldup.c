#include "spm.h"
#include "framework.h"

const char *testFmt = "case: (array){%s} expected '%s', returned '%s' \n";
struct TestCase testCase[] = {
        {.caseValue.slptr = (char *[]){"a", "a", "c", "d", "e", NULL}, .truthValue.sptr = "acde"},
        {.caseValue.slptr = (char *[]){"a", "b", "b", "d", "e", NULL}, .truthValue.sptr = "abde"},
        {.caseValue.slptr = (char *[]){"a", "b", "c", "c", "e", NULL}, .truthValue.sptr = "abce"},
        {.caseValue.slptr = (char *[]){"a", "b", "c", "d", "e", NULL}, .truthValue.sptr = "abcde"},
        {.caseValue.slptr = (char *[]){"e", "d", "c", "b", "a", NULL}, .truthValue.sptr = "edcba"},
        {.caseValue.slptr = NULL, .truthValue.sptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char **result = strdeldup(testCase[i].caseValue.slptr);
        if (testCase[i].truthValue.sptr == NULL && result == NULL) {
            continue;
        } else if (testCase[i].truthValue.sptr == NULL && result != NULL) {
            fprintf(stderr, testFmt, array_to_string(testCase[i].caseValue.slptr, ", "), testCase[i].truthValue.sptr, array_to_string(result, ""));
            return 1;
        }
        char *str_result = join(result, "");

        myassert(strcmp(str_result, testCase[i].truthValue.sptr) == 0, testFmt, array_to_string(testCase[i].caseValue.slptr, ", "), testCase[i].truthValue.sptr, str_result);

        free(str_result);
    }
    return 0;
}