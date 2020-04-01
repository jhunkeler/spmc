#include "spm.h"
#include "framework.h"

const char *mockConfig[] = {
        (const char *){"\n"  // empty line
         "; comment_semicolon\n"
         "# comment_hash\n"
         "string_unquoted = unquoted string\n"
         "string_quoted = \"quoted string\"\n"
         "inline_comment_semicolon = inline semicolon comment ; inline_comment_semicolon\n"
         "inline_comment_hash = inline hash comment # inline_comment_hash\n"
         "whitespace = \t    ignored\t          \n"
         "integer = 1\n"
         "decimal = 1.0\n"
         ""},
        (const char *){
            "oh\n"
            "no\n"
            "=\n"
            "extreme failure\n",
        },
        NULL, // end
};

#define SETARG_UINT(ARGINDEX, VALUE) .arg[ARGINDEX].unsigned_integer = VALUE
#define GETARG_UINT(CASE, ARGINDEX) CASE.arg[ARGINDEX].unsigned_integer

const char *testFmt = "case '%s': returned '%s', expected '%s'\n";
const char *testSizesFmt = "case '%s': returned '%zu', expected '%zu'\n";
struct TestCase testCase[] = {
    {.caseValue.sptr = "string_unquoted", .truthValue.sptr = "unquoted string", SETARG_UINT(0, 15), SETARG_UINT(1, 15)},
    {.caseValue.sptr = "string_quoted", .truthValue.sptr = "quoted string", SETARG_UINT(0, 13), SETARG_UINT(1, 13)},
    {.caseValue.sptr = "inline_comment_semicolon", .truthValue.sptr = "inline semicolon comment", SETARG_UINT(0, 24), SETARG_UINT(1, 24)},
    {.caseValue.sptr = "inline_comment_hash", .truthValue.sptr = "inline hash comment", SETARG_UINT(0, 19), SETARG_UINT(1, 19)},
    {.caseValue.sptr = "whitespace", .truthValue.sptr = "ignored", SETARG_UINT(0, 10), SETARG_UINT(1, 7)},
    {.caseValue.sptr = "integer", .truthValue.sptr = "1", SETARG_UINT(0, 7), SETARG_UINT(1, 1)},
    {.caseValue.sptr = "decimal", .truthValue.sptr = "1.0", SETARG_UINT(0, 7), SETARG_UINT(1, 3)},
    {.caseValue.sptr = "missing", .truthValue.sptr = NULL, SETARG_UINT(0, 0), SETARG_UINT(1, 0)},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    ConfigItem **config = NULL;
    char filename[NAME_MAX];

    memset(filename, '\0', sizeof(filename));
    sprintf(filename, "%s.%s.mock", basename(__FILE__), __FUNCTION__);

    mock(filename, (void *) mockConfig[0], sizeof(char), strlen(mockConfig[0]));
    config = config_read(filename);

    for (size_t i = 0; i < numCases; i++) {
        ConfigItem *item = NULL;
        const char *caseValue = testCase[i].caseValue.sptr;
        const char *truthValue = testCase[i].truthValue.sptr;

        item = config_get(config, caseValue);
        if (testCase[i].truthValue.sptr == NULL) {
            myassert(item == NULL, "item was not NULL\n");
            continue;
        }
        myassert(strcmp(caseValue, item->key) == 0, testFmt, caseValue, item->key, caseValue);
        myassert(strcmp(truthValue, item->value) == 0, testFmt, caseValue, item->value, truthValue);
        myassert(GETARG_UINT(testCase[i], 0) == item->key_length, testSizesFmt, caseValue, item->key_length, GETARG_UINT(testCase[i], 0));
        myassert(GETARG_UINT(testCase[i], 1) == item->value_length, testSizesFmt, caseValue, item->value_length, GETARG_UINT(testCase[i], 1));
    }

    config_free(config);
    unlink(filename);

    // Now throw some bad input data at it
    mock(filename, (void *) mockConfig[1], sizeof(char), strlen(mockConfig[1]));
    config = config_read(filename);
    myassert(config == NULL, "input data was invalid and config_read(\"%s\") did not return NULL.", filename);

    config_free(config);
    unlink(filename);

    return 0;
}