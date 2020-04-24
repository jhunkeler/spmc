#include "spm.h"
#include "framework.h"

struct TestCase testCase[] = {
        {.caseValue.sptr = "/bin/sh", .truthValue.signed_integer = 0},
        {.caseValue.sptr = "/usr/bin/tar", .truthValue.signed_integer = 0},
        {.caseValue.sptr = "/dev/null", .truthValue.signed_integer = -1}, // not an object
        {.caseValue.sptr = NULL, .truthValue.signed_integer = -1}, // invalid call
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        StrList *result = shlib_deps(testCase[i].caseValue.sptr);
        if (result == NULL && testCase[i].truthValue.signed_integer < 0 && spmerrno == EINVAL) {
            // expected failure
            continue;
        }

        myassert(spmerrno == 0, "case %zu: raised unhandled exception %d: %s\n", i, spmerrno, spm_strerror(spmerrno));
        myassert(result != NULL, "case %zu: unexpected NULL", i);
        for (size_t j = 0; j < strlist_count(result);  j++) {
            char *item = strlist_item(result, j);
            myassert(exists(item) == testCase[i].truthValue.signed_integer,
                    "library record found, but does not exist: '%s' (your OS is broken)\n", item);
        }
        strlist_free(result);
    }
    return 0;
}