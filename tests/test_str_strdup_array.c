#include "spm.h"
#include "framework.h"

const char *testFmt = "case: (array){%s} expected (array){%s}, returned (array){%s} \n";
struct TestCase testCase[] = {
        {.caseValue.slptr = (char *[]){"a", "a", "c", "d", "e", NULL}, .truthValue.slptr = (char *[]){"a", "a", "c", "d", "e", NULL}},
        {.caseValue.slptr = (char *[]){"e", "d", "c", "b", "a", NULL}, .truthValue.slptr = (char *[]){"e", "d", "c", "b", "a", NULL}},
        {.caseValue.slptr = (char *[]){"ha", "haha", "hahaha", "hahahaha", "hahahahaha", NULL}, .truthValue.slptr = (char *[]){"ha", "haha", "hahaha", "hahahaha", "hahahahaha", NULL}},
        {.caseValue.slptr = NULL, .truthValue.sptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char **result = strdup_array(testCase[i].caseValue.slptr);
        myassert(strcmp_array(result, testCase[i].truthValue.slptr) == 0,
                testFmt,
                array_to_string(testCase[i].caseValue.slptr, ", "),
                array_to_string(testCase[i].truthValue.slptr, ", "),
                array_to_string(result, ", "));

        if (result != NULL) {
            for (size_t r = 0; result[r] != NULL; r++) {
                free(result[r]);
            }
            free(result);
        }
    }
    return 0;
}