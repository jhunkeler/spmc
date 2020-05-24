#include "spm.h"
#include "framework.h"


const char *testFmt = "case %s: returned %llu (0x%llX), expected >%llu (0x%llX)\n";
struct TestCase testCase[] = {
        {},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

char *versions[] = {
        "0.0.1", "0.0.2", "0.0.3", "0.0.4", "0.0.5", "0.0.6", "0.0.7", "0.0.8", "0.0.9", "0.0.10",
        "0.1", "0.1.1", "0.1.2", "0.1.3", "0.1.4", "0.1.5", "0.1.6", "0.1.7", "0.1.8", "0.1.9", "0.1.10",
        "0.2", "0.2.1", "0.2.2", "0.2.3", "0.2.4", "0.2.5", "0.2.6", "0.2.7", "0.2.8", "0.2.9", "0.2.10",
        "0.9", "0.9.1", "0.9.2", "0.9.3", "0.9.4", "0.9.5", "0.9.6", "0.9.7", "0.9.8", "0.9.9", "0.9.10",
        "0.10.0", "0.10.1", "0.10.2", "0.10.3", "0.10.4", "0.10.5", "0.10.6", "0.10.7", "0.10.8", "0.10.9",
        "0.10.10",
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
        "100.1a", "100.1a2", "100.3a10", "101", "101.1",
        "2019.1", "2019.2a", "2019.3",
        "2020.1", "2020.2a", "2020.3", "2020.4.1",
};
char *nonsense[] = {
        "big_fat_sausage_waffles+glass_of_juice",
        "big_fat_sausage_waffles.1+cup_of_fresh_broken_glass",
};

int main(int argc, char *argv[]) {
    struct Version *prev = NULL;

    for (size_t i = 0; i < (sizeof(versions) / sizeof(char *)); i++) {
        struct Version *version = version_init();
        version_read(&version, versions[i]);

        myassert(spmerrno == 0, "Unexpected error: %s\n", spm_strerror(spmerrno));

        if (prev != NULL) {
            myassert(version->asInt > prev->asInt,
                     testFmt, versions[i],
                     version->asInt, version->asInt,
                     prev->asInt, prev->asInt);
        }

        prev = version;
    }

    prev = NULL;
    for (size_t i = 0; i < (sizeof(nonsense) / sizeof(char *)); i++) {
        struct Version *version = version_init();
        version_read(&version, nonsense[i]);

        myassert(spmerrno == 0, "Unexpected error: %s\n", spm_strerror(spmerrno));

        if (prev != NULL) {
            myassert(version->asInt > prev->asInt,
                     testFmt, nonsense[i],
                     version->asInt, version->asInt,
                     prev->asInt, prev->asInt);
        }

        prev = version;
    }

    // Check the wrapper, version_from(), produces the same results as "the long way" above
    uint64_t prev_i = 0;
    for (size_t i = 0; i < (sizeof(versions) / sizeof(char *)); i++) {
        myassert(spmerrno == 0, "Unexpected error: %s\n", spm_strerror(spmerrno));

        uint64_t result = version_from(versions[i]);
        if (prev_i > 0) {
            myassert(result > prev_i,
                     testFmt, versions[i],
                     result, result,
                     prev_i, prev_i);
        }

        prev_i = result;
    }

    prev_i = 0;
    for (size_t i = 0; i < (sizeof(nonsense) / sizeof(char *)); i++) {
        myassert(spmerrno == 0, "Unexpected error: %s\n", spm_strerror(spmerrno));

        uint64_t result = version_from(nonsense[i]);
        if (prev_i > 0) {
            myassert(result > prev_i,
                     testFmt, nonsense[i],
                     result, result,
                     prev_i, prev_i);
        }

        prev_i = result;
    }
    return 0;
}
