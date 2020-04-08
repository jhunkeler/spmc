#include "spm.h"
#include "framework.h"

const char *testFmt = "case '%s': returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.caseValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF", .arg[0].sptr = "...", .arg[1].sptr = ".", .truthValue.sptr = "\xFE\xFEThis is a test. Happy friend.\x00\xFF\xFF"},
        {.caseValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF", .arg[0].sptr = "test", .arg[1].sptr = "win", .truthValue.sptr = "\xFE\xFEThis is a win... Happy friend.\x00\xFF\xFF"},
        {.caseValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF", .arg[0].sptr = "This is a test", .arg[1].sptr = "Meow cat", .truthValue.sptr = "\xFE\xFEMeow cat... Happy friend.\x00\xFF\xFF"},
        {.caseValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF", .arg[0].sptr = "T", .arg[1].sptr = "#", .truthValue.sptr = "\xFE\xFE#his is a test... Happy friend.\x00\xFF\xFF"},
        {.caseValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF", .arg[0].sptr = "", .arg[1].sptr = "#", .truthValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF"},
        {.caseValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF", .arg[0].sptr = "#", .arg[1].sptr = "", .truthValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF"},
        {.caseValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF", .arg[0].sptr = "", .arg[1].sptr = "", .truthValue.sptr = "\xFE\xFEThis is a test... Happy friend.\x00\xFF\xFF"},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        FILE *fp = NULL;
        char buffer[BUFSIZ] = {0,};
        char *caseValue = buffer;
        int return_value = 0;
        char filename[PATH_MAX] = {0,};
        sprintf(filename, "%s.%s_%zu.mock", basename(__FILE__), __FUNCTION__, i);

        mock(filename, (void *)testCase[i].caseValue.sptr, sizeof(char), strlen(testCase[i].caseValue.sptr));
        return_value = relocate(filename, testCase[i].arg[0].sptr, testCase[i].arg[1].sptr);

        fp = fopen(filename, "rb");
        if (fp == NULL) {
            perror(filename);
            exit(1);
        }

        if (fread(caseValue, sizeof(char), BUFSIZ, fp) == 0) {
            fprintf(stderr, "%s: read failure\n", filename);
            fclose(fp);
            exit(1);
        }

        myassert(return_value == 0, "reloc exited with error condition: %d\n", return_value);
        myassert(strstr(caseValue, testCase[i].truthValue.sptr) != NULL, testFmt, testCase[i].caseValue.sptr, caseValue, testCase[i].truthValue.sptr);

        // clean up
        unlink(filename);
    }
    return 0;
}