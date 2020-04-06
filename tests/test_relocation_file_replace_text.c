#include "spm.h"
#include "framework.h"

const char *testFmt = "case '%s': returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.caseValue.sptr = "This is a test... Happy friend.", .arg[0].sptr = "...", .arg[1].sptr = ".", .truthValue.sptr = "This is a test. Happy friend."},
        {.caseValue.sptr = "This is a test... Happy friend.", .arg[0].sptr = "test", .arg[1].sptr = "win", .truthValue.sptr = "This is a win... Happy friend."},
        {.caseValue.sptr = "This is a test... Happy friend.", .arg[0].sptr = "This is a test", .arg[1].sptr = "Meow cat", .truthValue.sptr = "Meow cat... Happy friend."},
        {.caseValue.sptr = "This is a test... Happy friend.", .arg[0].sptr = "T", .arg[1].sptr = "#", .truthValue.sptr = "#his is a test... Happy friend."},
        {.caseValue.sptr = "This is a test... Happy friend.", .arg[0].sptr = "", .arg[1].sptr = "#", .truthValue.sptr = "This is a test... Happy friend."},
        {.caseValue.sptr = "This is a test... Happy friend.", .arg[0].sptr = "#", .arg[1].sptr = "", .truthValue.sptr = "This is a test... Happy friend."},
        {.caseValue.sptr = "This is a test... Happy friend.", .arg[0].sptr = "", .arg[1].sptr = "", .truthValue.sptr = "This is a test... Happy friend."},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char filename[PATH_MAX] = {0,};
        sprintf(filename, "%s.%s_%zu.mock", basename(__FILE__), __FUNCTION__, i);

        mock(filename, testCase[i].caseValue.sptr, sizeof(char), strlen(testCase[i].caseValue.sptr));
        file_replace_text(filename, testCase[i].arg[0].sptr, testCase[i].arg[1].sptr);

        char **data = file_readlines(filename, 0, 0, NULL);
        char *caseValue = join(data, "");

        myassert(strcmp(caseValue, testCase[i].truthValue.sptr) == 0, testFmt, testCase[i].caseValue.sptr, caseValue, testCase[i].truthValue.sptr);

        free(caseValue);

        if (access(filename, X_OK) == 0) {
            unlink(filename);
        }
    }
    return 0;
}