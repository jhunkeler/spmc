#include "spm.h"
#include "framework.h"

struct TestCase testCase[] = {
        {.arg[0].sptr = "testing", .arg[1].sptr = NULL},
        {.arg[0].sptr = "testing", .arg[1].sptr = "a/sub_directory/here"},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *result = spm_mkdtemp(".", testCase[i].arg[0].sptr, testCase[i].arg[2].sptr);
        myassert(result != NULL, "unexpected NULL on case %zu\n", i);
        int present = access(result, F_OK);
        myassert(present == 0, "%s: %s\n", result, strerror(errno));
        rmdirs(result);
        free(result);
    }
    free(TMP_DIR);
    return 0;
}
