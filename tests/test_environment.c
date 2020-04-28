#include "spm.h"
#include "framework.h"

const char *testFmt = "returned '%s', expected '%s'\n";
struct TestCase testCase = {
        .caseValue.voidptr = (char *[]){"test1=one", "test2=two", NULL},
        .inputValue.voidptr = (char *[]){"test1", "test2", NULL},
        .truthValue.voidptr = (char *[]){"one", "two", NULL},
        .arg[0].sptr = "one hundred",
};

int main(int argc, char *argv[]) {
    RuntimeEnv *rt = NULL;
    char **pInput = (char **)testCase.inputValue.voidptr;
    char **pTruth = (char **)testCase.truthValue.voidptr;

    rt = runtime_copy((char **)testCase.caseValue.voidptr);
    myassert(rt != NULL, "runtime_copy failed");

    // Are the keys we just inserted actually there?
    for (size_t j = 0; pInput[j] != NULL; j++) {
        char *result = runtime_get(rt, pInput[j]);
        myassert(strcmp(result, pTruth[j]) == 0, "returned '%s', expected '%s'\n", result, pTruth[j]);
    }

    // Set a key that already exists to have a different value
    runtime_set(rt, pInput[0], testCase.arg[0].sptr);
    myassert(strcmp(runtime_get(rt, pInput[0]), testCase.arg[0].sptr) == 0,
            "runtime_set changed contents of '%s', but did not work: '%s'\n", runtime_get(rt, pInput[0]), runtime_get(rt, pInput[0]));

    myassert(runtime_contains(rt, pInput[0]) >= 0, "key '%s' is not present", pInput[0]);
    myassert(runtime_contains(rt, pInput[1]) >= 0, "key '%s' is not present", pInput[1]);

    // Apply changes in `rt` to system environment
    runtime_apply(rt);
    myassert(getenv(pInput[0]) != NULL, "runtime_apply failed");
    myassert(getenv(pInput[1]) != NULL, "runtime_apply failed");

    runtime_free(rt);
}