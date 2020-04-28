#include "spm.h"
#include "framework.h"

const char *testFmt = "case: (array){%s}: returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.inputValue.unsigned_int = SPM_SORT_NUMERIC, .caseValue.slptr = (char *[]){"1", "100", "10", "10000", "1000", "0", NULL}, .truthValue.sptr = "0 1 10 100 1000 10000"},
        {.inputValue.unsigned_int = SPM_SORT_LEN_ASCENDING, .caseValue.slptr = (char *[]){"1", "100", "10", "10000", "1000", "0", NULL}, .truthValue.sptr = "1 0 10 100 1000 10000"}, // no QA?
        {.inputValue.unsigned_int = SPM_SORT_LEN_DESCENDING, .caseValue.slptr = (char *[]){"1", "100", "10", "10000", "1000", "0", NULL}, .truthValue.sptr = "10000 1000 100 10 1 0"},
        {.inputValue.unsigned_int = SPM_SORT_ALPHA, .caseValue.slptr = (char *[]){"c", "b", "a", NULL}, .truthValue.sptr = "a b c"},
        {.inputValue.unsigned_int = SPM_SORT_ALPHA, .caseValue.slptr = (char *[]){"a", "b", "c", NULL}, .truthValue.sptr = "a b c"},
        {.inputValue.unsigned_int = SPM_SORT_ALPHA, .caseValue.slptr = (char *[]){"3", "2", "1", NULL}, .truthValue.sptr = "1 2 3"},
        {.inputValue.unsigned_int = SPM_SORT_ALPHA, .caseValue.slptr = (char *[]){"1", "2", "3", NULL}, .truthValue.sptr = "1 2 3"},
        {.inputValue.unsigned_int = SPM_SORT_ALPHA, .caseValue.slptr = (char *[]){"package2", "package1", NULL}, .truthValue.sptr = "package1 package2"},
        {.inputValue.unsigned_int = SPM_SORT_ALPHA, .caseValue.slptr = (char *[]){"package1", "package2", NULL}, .truthValue.sptr = "package1 package2"},
        {.inputValue.unsigned_int = SPM_SORT_ALPHA, .caseValue.sptr = NULL, .truthValue.slptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char **array_orig = strdup_array(testCase[i].caseValue.slptr);
        char **array_case = strdup_array(array_orig);

        strsort(array_case, testCase[i].inputValue.unsigned_int);
        char *str_result = join(array_case, " ");

        if (testCase[i].truthValue.sptr == NULL && str_result == NULL) {
            continue;
        }
        myassert(strcmp(str_result, testCase[i].truthValue.sptr) == 0, testFmt, array_to_string(array_orig, ", "), str_result, testCase[i].truthValue.sptr);
        free(str_result);
        split_free(array_case);
        split_free(array_orig);
    }
    return 0;
}