#include "spm.h"
#include "framework.h"

#define FILENAME "touched_file"

const char *testFmt = "case: '%s': returned '%d', expected '%d'\n";
struct TestCase testCase[] = {
        {.caseValue.sptr = FILENAME, .truthValue.signed_int = 0},   // create file
        {.caseValue.sptr = FILENAME, .truthValue.signed_int = 0},   // update file
        {.caseValue.sptr = FILENAME, .truthValue.signed_int = 0},   // update file
        {.caseValue.sptr = ".", .truthValue.signed_int = -1},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

static void cleanup() {
    if (access(FILENAME, F_OK) == 0) {
        unlink(FILENAME);
    }
}

int main(int argc, char *argv[]) {
    cleanup();

    for (size_t i = 0; i < numCases; i++) {
        int result = touch(testCase[i].caseValue.sptr);
        myassert(result == testCase[i].truthValue.signed_int, testFmt, testCase[i].caseValue.sptr, result, testCase[i].truthValue.signed_int);
    }

    cleanup();
    return 0;
}