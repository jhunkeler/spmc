#include "spm.h"
#include "framework.h"

const char *testFmt = "could not split '%s' with '%s'\n";
struct TestCase testCase[] = {
        {.inputValue.sptr = " ", .caseValue.str = "one two three", .truthValue.slptr = (char *[]){"one", "two", "three", NULL}},
        {.inputValue.sptr = "|", .caseValue.str = "one|two|three", .truthValue.slptr = (char *[]){"one", "two", "three", NULL}},
        {.inputValue.sptr = "a", .caseValue.str = "123a456a789", .truthValue.slptr = (char *[]){"123", "456", "789", NULL}},
        {.inputValue.sptr = "b", .caseValue.str = "123a456b789", .truthValue.slptr = (char *[]){"123a456", "789", NULL}},
        {.inputValue.sptr = NULL, .caseValue.str = "123a456a789", .truthValue.slptr = NULL},
        {.inputValue.sptr = "b", .caseValue.sptr = NULL, .truthValue.slptr = NULL},
        {.inputValue.sptr = NULL, .caseValue.sptr = NULL, .truthValue.slptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char **result = split(testCase[i].caseValue.str, testCase[i].inputValue.sptr);
        if (testCase[i].truthValue.slptr == NULL && result == NULL) {
            continue;
        }

        for (size_t part = 0; testCase[i].truthValue.slptr[part] != NULL; part++) {
            myassert(strcmp(result[part], testCase[i].truthValue.slptr[part]) == 0, testFmt, testCase[i].caseValue.str, testCase[i].inputValue.sptr);
        }
        split_free(result);
    }
    return 0;
}