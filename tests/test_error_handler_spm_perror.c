#include "spm.h"
#include "framework.h"

const char *testFmt = "case %s: returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.caseValue.sptr = "oh no it broke", .truthValue.sptr = "oh no it broke: No such file or directory", .arg[0].signed_int = ENOENT},
        {.caseValue.sptr = "kaboom", .truthValue.sptr = "kaboom: Failed to fetch package", .arg[0].signed_int = SPM_ERR_PKG_FETCH},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    char *tty_path = NULL;
    if ((tty_path = ttyname(STDERR_FILENO)) == NULL) {
        perror("tty check");
        return 0;
    }

    for (size_t i = 0; i < numCases; i++) {
        char buf[BUFSIZ] = {0,};
        char *buffer = buf;
        int stderr_save = 0;

        fflush(stderr);

        // Redirect stdout
        stderr_save = dup(STDERR_FILENO);
        freopen("/dev/null", "w", stderr);
        setvbuf(stderr, buffer, _IOLBF, sizeof(buf));

        // Do test
        spmerrno = testCase[i].arg[0].signed_int;
        spm_perror(testCase[i].caseValue.sptr);
        fflush(stderr);

        strip(buffer);

        // Restore stderr
        dup2(stderr_save, STDERR_FILENO);
        setvbuf(stderr, NULL, _IONBF, 0);

        myassert(strcmp(buffer, testCase[i].truthValue.sptr) == 0, testFmt, testCase[i].caseValue.sptr, buffer, testCase[i].truthValue.sptr);
    }
}
