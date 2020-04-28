#include "spm.h"
#include "framework.h"

static const char *story_truth = "The quick brown fox jumps over the lazy dog.";
static const char *story_truth_rev = "dog. jumps over the lazy fox The quick brown";
static const char *story_truth_sort_asc = "fox dog. The quick brown jumps over the lazy";
static const char *story_truth_sort_dsc = "jumps over the lazy The quick brown dog. fox";

static char *DATA[] = {
        "The quick brown",
        "fox",
        "jumps over the lazy",
        "dog.",
        NULL,
};

long int MAX_INTS[] = {
        INT8_MAX,
        UINT8_MAX,
        INT16_MAX,
        UINT16_MAX,
        INT32_MAX,
        UINT32_MAX,
        INT64_MAX,
        UINT64_MAX,
        0,
};

enum truthValue {
    int8_max = 0,
    uint8_max,
    int16_max,
    uint16_max,
    int32_max,
    uint32_max,
    int64_max,
    uint64_max,
};


int main(int argc, char *argv[]) {
    const char *storyFmt = "expected story: '%s', but got '%s'\n";
    char *story = NULL;
    union TestValue testValue = {0,};
    StrList *strList = NULL;
    StrList *strListCopy = NULL;
    StrList *strListNumbers = NULL;
    StrList truthInit = {1, 0, NULL};
    size_t used = 0;
    size_t allocated = 0;
    char intStr[255];
    const int DATA_SIZE = (sizeof(DATA) / sizeof(char *)) - 1;


    // Initialize string list and check initial state
    strList = strlist_init();
    myassert(strList->num_inuse == 0, "strList has wrong number of records in use: %zu (expected %zu)\n", strList->num_inuse, truthInit.num_inuse);
    myassert(strList->num_alloc == 1, "strList has wrong number of records allocated: %zu (expected %zu)\n", strList->num_alloc, truthInit.num_alloc);
    myassert(strList->data != NULL, "strList was NULL");

    ssize_t count = strlist_count(strList);
    myassert(count == 0, "strlist_count returned wrong number of records in use: %zu (expected %zu)\n", count, strList->num_inuse);

    // Populate list with strings
    for (size_t i = 0; DATA[i] != NULL; i++) {
        used = i + 1;
        allocated = used + 1;

        strlist_append(strList, DATA[i]);
        myassert(strList->num_inuse == used, "incorrect used record count post-append\n");
        myassert(strList->num_alloc == allocated, "incorrect allocated record count post-append\n");
    }

    // Is the data represented in the array as we expect it to be?
    story = join(strList->data, " ");
    myassert(strcmp(story, story_truth) == 0, storyFmt, story_truth, story);
    free(story);
    story = NULL;

    // Copy the array (because we're about to modify it)
    strListCopy = strlist_copy(strList);
    myassert(strListCopy != NULL, "strlist_copy failed\n");

    // Does reversing the array work correctly?
    strlist_reverse(strListCopy);

    story = join(strListCopy->data, " ");
    // Copy the array (because we're about to modify it)
    myassert(strcmp(story, story_truth_rev) == 0, storyFmt, story_truth, story);
    free(story);
    story = NULL;
    strlist_free(strListCopy);
    strListCopy = NULL;

    // Copy the array (because we're about to modify it)
    strListCopy = strlist_copy(strList);
    myassert(strListCopy != NULL, "strlist_copy failed\n");

    // Now compare the arrays to make sure they're identical
    myassert(strlist_cmp(strList, strListCopy) == 0, "strlist_copy result does not match original StrList contents");

    // Sort the array to see if it works.
    strlist_sort(strListCopy, SPM_SORT_LEN_ASCENDING);

    // The array just got modified, so check to make sure they are NOT identical
    myassert(strlist_cmp(strList, strListCopy) == 1, "StrList data matches original StrList contents (after modification)");

    story = join(strListCopy->data, " ");
    myassert(strcmp(story, story_truth_sort_asc) == 0, storyFmt, story_truth_sort_asc, story);
    free(story);
    story = NULL;
    strlist_free(strListCopy);
    strListCopy = NULL;

    // Copy the array (because we're about to modify it)
    strListCopy = strlist_copy(strList);
    myassert(strListCopy != NULL, "strlist_copy failed\n");

    // Sort the array once more using a different comparator
    strlist_sort(strListCopy, SPM_SORT_LEN_DESCENDING);

    story = join(strListCopy->data, " ");
    myassert(strcmp(story, story_truth_sort_dsc) == 0, storyFmt, story_truth_sort_dsc, story);
    free(story);
    story = NULL;
    strlist_free(strListCopy);
    strListCopy = NULL;

    // Now append numerical values (as string) so we can start reading them back
    for (size_t i = 0; MAX_INTS[i] != 0; i++) {
        memset(intStr, '\0', sizeof(intStr));
        sprintf(intStr, "%zu", MAX_INTS[i]);
        strlist_append(strList, intStr);
    }

    memset(intStr, '\0', sizeof(intStr));
    sprintf(intStr, "%lf", MAXFLOAT);
    strlist_append(strList, intStr);

    // Now make sure values derived from strlist_item_as_*() functions work properly
    // NOTE: My focus is on 64-bit, so if you're compiling this on a 32-bit computer
    //       and these tests fail for you... That's a shame.
    testValue.signed_char = strlist_item_as_char(strList, DATA_SIZE + int8_max);
    myassert(testValue.signed_char == INT8_MAX, "int8_max incorrect: %d", testValue.signed_char);

    testValue.unsigned_char = strlist_item_as_uchar(strList, DATA_SIZE + uint8_max);
    myassert(testValue.unsigned_char == UINT8_MAX, "uint8_max incorrect: %d", testValue.unsigned_char);

    testValue.signed_short = strlist_item_as_short(strList, DATA_SIZE + int16_max);
    myassert(testValue.signed_short == INT16_MAX, "int16_max incorrect: %d", testValue.signed_short);

    testValue.unsigned_short = strlist_item_as_ushort(strList, DATA_SIZE + uint16_max);
    myassert(testValue.unsigned_short == UINT16_MAX, "uint16_max incorrect: %d", testValue.unsigned_short);

    testValue.signed_int = strlist_item_as_int(strList, DATA_SIZE + int32_max);
    myassert(testValue.signed_int == INT32_MAX, "int32_max incorrect: %d", testValue.signed_int);

    testValue.unsigned_int = strlist_item_as_uint(strList, DATA_SIZE + uint32_max);
    myassert(testValue.unsigned_int == UINT32_MAX, "uint32_max incorrect: %d", testValue.unsigned_int);

    testValue.signed_long = strlist_item_as_long(strList, DATA_SIZE + int64_max);
    myassert(testValue.signed_long == INT64_MAX, "int64_max (long) incorrect: %ld", testValue.signed_long);

    testValue.unsigned_long = strlist_item_as_ulong(strList, DATA_SIZE + uint64_max);
    myassert(testValue.unsigned_long == UINT64_MAX, "uint64_max (long) incorrect: %lu", testValue.unsigned_long);

    testValue.signed_long_long = strlist_item_as_long_long(strList, DATA_SIZE + int64_max);
    myassert(testValue.signed_long_long == INT64_MAX, "int64_max (long long) incorrect: %lld", testValue.signed_long_long);

    testValue.unsigned_long_long = strlist_item_as_ulong_long(strList, DATA_SIZE + uint64_max);
    myassert(testValue.unsigned_long_long == UINT64_MAX, "uint64_max (long long) incorrect: %llu", testValue.unsigned_long_long);

    testValue.floating = strlist_item_as_float(strList, DATA_SIZE + uint64_max + 1);
    myassert(testValue.floating == MAXFLOAT, "floating point maximum incorrect: %f", testValue.floating);

    strlist_free(strList);
    return 0;
}