#include "spm.h"
#include "framework.h"

const char *testFmt = "returned '%s', expected '%s'\n";

int main(int argc, char *argv[]) {
    int retval_lib = 0;
    int retval_bin = 0;
    char *filename_lib = mock_image(AS_MOCK_LIB, "libwinning", (char *[]) {"-L/usr/lib", "-lz", NULL}, &retval_lib);
    char *filename_bin = mock_image(AS_MOCK_BIN, "winning", (char *[]) {"-L.", "-lwinning", NULL}, &retval_bin);
    char *rpath_lib = rpath_get(filename_lib);
    char *rpath_bin = rpath_get(filename_bin);

    myassert(retval_lib == 0, "mock library build failed with code: %d\n", retval_lib);
    myassert(strcmp(dirname(filename_lib), rpath_lib) == 0, testFmt, dirname(filename_lib), rpath_lib);

    myassert(retval_lib == 0, "mock executable build failed with code: %d\n", retval_bin);
    myassert(strcmp(dirname(filename_bin), rpath_bin) == 0, testFmt, dirname(filename_bin), rpath_bin);

    return 0;
}