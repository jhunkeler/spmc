#include "spm.h"
#include "framework.h"

#define KILOBYTE 1024

const char *testFmt = "returned '%zu', expected '%zu'\n";
struct TestCase testCase[] = {
        {.caseValue.unsigned_int = 0, .truthValue.unsigned_int = 0},
        {.caseValue.unsigned_int = 1, .truthValue.unsigned_int = 1},
        {.caseValue.unsigned_int = KILOBYTE, .truthValue.unsigned_int = KILOBYTE},
        {.caseValue.unsigned_int = KILOBYTE * 1024, .truthValue.unsigned_int = KILOBYTE * 1024},
        {.caseValue.unsigned_int = KILOBYTE * 1024 * 10, .truthValue.unsigned_int = KILOBYTE * 1024 * 10},
#if defined(TESTS_EXPENSIVE)
        {.caseValue.unsigned_int = KILOBYTE * 1024 * 100, .truthValue.unsigned_int = KILOBYTE * 1024 * 100},
#endif
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    const char *fill = "$";
    for (size_t i = 0; i < numCases; i++) {
        char *filename = mock_size(testCase[i].caseValue.unsigned_int, fill);
        size_t result = get_file_size(filename);
        myassert(result == testCase[i].truthValue.unsigned_int, testFmt, result, testCase[i].truthValue.unsigned_int);
        unlink(filename);
    }
    return 0;
}