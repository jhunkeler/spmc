#include "spm.h"
#include "framework.h"

struct TestCase testCase[] = {
        {.arg[0].sptr = "fslist/testdir", .arg[1].sptr = "testfile", .arg[2].sptr = "testlink"},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        char *filename = NULL;
        char *linkname = NULL;
        char *dirnam = NULL;
        FSList *listing = NULL;

        if (startswith(testCase[i].arg[0].sptr, DIRSEPS)) {
            fprintf(stderr, "INSECURE TEST CASE: '%s' (starts with, or is, '%s')\n", testCase[i].arg[0].sptr, DIRSEPS);
            exit(2);
        }

        // Clean previous run
        rmdirs(dirname(testCase[i].arg[0].sptr));

        // Create test case directory
        mkdirs(testCase[i].arg[0].sptr, 0755);

        // Render paths
        filename = join((char *[]){testCase[i].arg[0].sptr, testCase[i].arg[1].sptr, NULL}, DIRSEPS);
        linkname = join((char *[]){testCase[i].arg[0].sptr, testCase[i].arg[2].sptr, NULL}, DIRSEPS);
        dirnam = join((char *[]){testCase[i].arg[0].sptr, basename(testCase[i].arg[0].sptr), NULL}, DIRSEPS);

        // Create a file
        if (touch(filename) < 0) {
            perror(filename);
            exit(1);
        }

        // Create a symlink (to file ^)
        if (symlink(basename(filename), linkname) < 0) {
            perror(linkname);
            exit(1);
        }

        // Create a directory
        if (mkdir(dirnam, 0755) < 0) {
            perror(dirnam);
            exit(1);
        }

        // Populate directory listing
        listing = fslist(testCase[i].arg[0].sptr);

        // Check FSList structure
        myassert(listing != NULL, "fslist() return NULL\n");
        myassert(strcmp(listing->root, testCase[i].arg[0].sptr) == 0, "listing->root points to '%s', instead of '%s'", listing->root, testCase[i].arg[0].sptr);
        myassert(listing->records > 0, "listing->records should be: >0 (was: %zu)\n", listing->records);
        myassert(listing->_num_alloc > listing->records, "listing->_num_alloc should be: >%zu (was: %zu)\n", listing->records, listing->_num_alloc);

        printf("root = %s\n", listing->root);
        // If this for-loop segfaults then the test fails
        for (size_t d = 0; d < listing->records; d++) {
            char type[NAME_MAX];
            switch (listing->record[d]->d_type) {
                case DT_DIR:
                    strcpy(type, "directory");
                    break;
                case DT_LNK:
                    strcpy(type, "symlink");
                    break;
                case DT_REG:
                    strcpy(type, "file");
                    break;
                default:
                    strcpy(type, "unknown");
                    break;
            }
            printf("%s[%zu] = %s\n", type, d, listing->record[d]->d_name);
        }

        // Clean up resources
        rmdirs(dirname(testCase[i].arg[0].sptr));
        fslist_free(listing);
    }
    return 0;
}