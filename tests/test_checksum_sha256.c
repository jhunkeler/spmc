#include "spm.h"
#include "shell.h"
#include "framework.h"

const char *testFmt = "returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
    {.caseValue.sptr = "check this hash", .truthValue.sptr = "72a9728d7b00d42fe573d91d023e2ebe4fa2c3c5d38c67306cbcd4f5e410aa33"},
    {.caseValue.sptr = "check this hash", .truthValue.sptr = "72a9728d7b00d42fe573d91d023e2ebe4fa2c3c5d38c67306cbcd4f5e410aa33"},
    {.caseValue.sptr = "check this hash", .truthValue.sptr = "72a9728d7b00d42fe573d91d023e2ebe4fa2c3c5d38c67306cbcd4f5e410aa33"},
    {.caseValue.sptr = "check this hash", .truthValue.sptr = "72a9728d7b00d42fe573d91d023e2ebe4fa2c3c5d38c67306cbcd4f5e410aa33"},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    char filename[NAME_MAX] = {0};
    sprintf(filename, "%s.%s.mock", basename(__FILE__), __FUNCTION__);

    for (size_t i = 0; i < numCases; i++) {
        char *result = NULL;
        char *data = strdup(testCase[i].caseValue.sptr);

        mock(filename, data, sizeof(char), strlen(testCase[i].caseValue.sptr));
        result = sha256sum(filename);

        myassert(strcmp(result, testCase[i].truthValue.sptr) == 0, testFmt, result, testCase[i].truthValue.sptr);

        free(result);
        free(data);
        unlink(filename);
    }
    return 0;
}