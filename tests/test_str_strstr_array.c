#include "spm.h"
#include "framework.h"

const char *testFmt = "case: (array){%s} expected '%s', returned '%s' \n";
struct TestCase testCase[] = {
        {.inputValue.sptr = "abracadabra", .caseValue.slptr = (char *[]){"abracadabra", "brisket", "gumball", NULL}, .truthValue.sptr = "abracadabra"},
        {.inputValue.sptr = "brisket", .caseValue.slptr = (char *[]){"abracadabra", "brisket", "gumball", NULL}, .truthValue.sptr = "brisket"},
        {.inputValue.sptr = "gumball", .caseValue.slptr = (char *[]){"abracadabra", "brisket", "gumball", NULL}, .truthValue.sptr = "gumball"},
        {.inputValue.sptr = "abrac", .caseValue.slptr = (char *[]){"abracadabra", "brisket", "gumball", NULL}, .truthValue.sptr = "abracadabra"},
        {.inputValue.sptr = "sket", .caseValue.slptr = (char *[]){"abracadabra", "brisket", "gumball", NULL}, .truthValue.sptr = "brisket"},
        {.inputValue.sptr = "ball", .caseValue.slptr = (char *[]){"abracadabra", "brisket", "gumball", NULL}, .truthValue.sptr = "gumball"},
        {.inputValue.sptr = NULL, .caseValue.slptr = NULL, .truthValue.sptr = NULL},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *result = strstr_array(testCase[i].caseValue.slptr, testCase[i].inputValue.sptr);
        if (testCase[i].truthValue.sptr == NULL && result == NULL) {
            continue;
        } else if (testCase[i].truthValue.sptr == NULL && result != NULL) {
            fprintf(stderr, testFmt, testCase[i].inputValue.str, testCase[i].truthValue.sptr, result);
            return 1;
        }
        myassert(strcmp(result, testCase[i].truthValue.sptr) == 0, testFmt, array_to_string(testCase[i].caseValue.slptr, ", "), testCase[i].truthValue.sptr, result);
    }
    return 0;
}