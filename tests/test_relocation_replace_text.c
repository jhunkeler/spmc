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
        char *caseValue = strdup(testCase[i].caseValue.sptr);
        size_t caseLen = strlen(caseValue);

        puts("BEFORE:");
        hexdump(caseValue, caseLen);
        print_banner("-", 79);
        size_t diff = replace_text(caseValue, testCase[i].arg[0].sptr, testCase[i].arg[1].sptr);
        printf("AFTER: (diff: %zu)\n", diff);
        hexdump(caseValue, caseLen);
        print_banner("-", 79);

        myassert(strcmp(caseValue, testCase[i].truthValue.sptr) == 0, testFmt, testCase[i].caseValue.sptr, caseValue, testCase[i].truthValue.sptr);
        free(caseValue);
    }
    return 0;
}