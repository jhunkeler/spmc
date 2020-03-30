#include "spm.h"
#include "shell.h"
#include "framework.h"

const char *testFmt = "returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.arg[0].unsigned_integer = SHELL_OUTPUT|SHELL_BENCHMARK, .arg[1].sptr = "echo hello; sleep 3", .arg[2].floating = 3, .arg[3].sptr = "hello"},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char elapsed[100] = {0,};
        Process *result = NULL;

        shell(&result, testCase[i].arg[0].unsigned_integer, testCase[i].arg[1].sptr);
        sprintf(elapsed, "%0.8lf", result->time_elapsed);
        strip(result->output);

        myassert(strcmp(result->output, testCase[i].arg[3].sptr) == 0, testFmt, result->output, testCase[i].arg[3].sptr);
        myassert(fabs(result->time_elapsed) >= fabs((double)testCase[i].arg[2].floating), testFmt, elapsed, "non-zero elapsed time");
    }
    return 0;
}