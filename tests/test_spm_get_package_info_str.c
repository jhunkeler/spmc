#include "spm.h"
#include "framework.h"

ManifestPackage TESTPKG = {.name = "test", .version = "1.0.0", .revision = "1", .archive = "test-1.0.0-1.tar.gz", .origin = "test_land", .size = 1234};
const char *testFmt = "returned '%s', expected '%s'\n";
char *truth[] = {
        "test 1.0.0 1 test-1.0.0-1.tar.gz test_land 1234 1.21K",
        "      test 1.0.0 1 test-1.0.0-1.tar.gz test_land 1234 1.21K",
        "test       1.0.0 1 test-1.0.0-1.tar.gz test_land 1234 1.21K",
        "test       1.0.0-1   ",
        "      test    1.0.0-1",
};
size_t numCases = 0;

int main(int argc, char *argv[]) {
    ManifestPackage *package = &TESTPKG;
    char *data[] = {
            spm_get_package_info_str(package, "%n %v %r %a %o %s %S"),
            spm_get_package_info_str(package, "%10n %v %r %a %o %s %S"),
            spm_get_package_info_str(package, "%-10n %v %r %a %o %s %S"),
            spm_get_package_info_str(package, "%-10n %-10V"),
            spm_get_package_info_str(package, "%10n %10V"),
    };
    numCases = sizeof(data) / sizeof(char *);

    if (numCases != (sizeof(truth) / sizeof(char *))) {
        fprintf(stderr, "Number of test cases does not match number of truth cases\n");
        return 1;
    }

    for (size_t i = 0; i < numCases; i++) {
        puts(data[i]);
        myassert(strcmp(data[i], truth[i]) == 0, testFmt, data[i], truth[i]);
        free(data[i]);
    }

    return 0;
}