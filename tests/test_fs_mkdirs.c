#include "spm.h"
#include "framework.h"

const char *testFmt = "returned '%d', expected '%d'\n";
struct TestCase testCase[] = {
        {.arg[0].str = "one", .truthValue.signed_integer = 0},
        {.arg[0].str = "one/two", .truthValue.signed_integer = 0},
        {.arg[0].str = "one/two/three", .truthValue.signed_integer = 0},
        {.arg[0].str = "one/two/three/four", .truthValue.signed_integer = 0},
        {.arg[0].str = "one/two/three/four/five", .truthValue.signed_integer = 0},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        int present = 0;
        int result = 0;
        char path_root[PATH_MAX] = {"test_fs_mkdirs_XXXXXX"};
        char *path = NULL;

        if (mkdtemp(path_root) == NULL) {
            perror("mkdtemp failed to create temporary directory");
            exit(errno);
        }

        path = join((char *[]){path_root, testCase[i].caseValue.str, NULL}, DIRSEPS);

        if ((result = mkdirs(path, 0755)) < 0) {
            perror(path);
            exit(1);
        }
        present = access(path, X_OK);

        myassert(result == 0, testFmt, result, testCase[i].truthValue.signed_integer);
        myassert(present == 0, testFmt, result, testCase[i].truthValue.signed_integer);

        rmdirs(path);

        present = access(path, X_OK);
        myassert(present != 0, testFmt, result, testCase[i].truthValue.signed_integer);
    }
    return 0;
}