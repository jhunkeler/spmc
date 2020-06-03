#include "spm.h"
#include "framework.h"

const char *testFmt = "returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {.arg[0].sptr = ".", .arg[1].slptr = NULL, .arg[2].unsigned_int = SPM_FSTREE_FLT_NONE},
        {.arg[0].sptr = ".", .arg[1].slptr = (char *[]){"world", NULL}, .arg[2].unsigned_int = SPM_FSTREE_FLT_CONTAINS},
        {.arg[0].sptr = ".", .arg[1].slptr = (char *[]){"/", NULL}, .arg[2].unsigned_int = SPM_FSTREE_FLT_STARTSWITH},
        {.arg[0].sptr = ".", .arg[1].slptr = (char *[]){".txt", NULL}, .arg[2].unsigned_int = SPM_FSTREE_FLT_ENDSWITH},
        {.arg[0].sptr = ".", .arg[1].slptr = (char *[]){"./hello", NULL}, .arg[2].unsigned_int = SPM_FSTREE_FLT_STARTSWITH | SPM_FSTREE_FLT_RELATIVE},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

int main(int argc, char *argv[]) {
    const char *hello_world = "hello_world.txt";
    char _tmpdir[PATH_MAX];
    char _startdir[PATH_MAX];

    char *tmpdir = _tmpdir;
    char *startdir = _startdir;

    // Create strings for local path information populated by the loop below
    char _tc_abspath[PATH_MAX] = {0};  // absolute path of working directory post-chdir
    char _cwd[PATH_MAX] = {0};  // working directory post-chdir

    // Create pointers to local path buffers
    char *tc_abspath = _tc_abspath;
    char *cwd = _cwd;

    // record the starting directory we executed this test from
    getcwd(startdir, PATH_MAX);

    for (size_t i = 0; i < numCases; i++) {
        // pointers to testCase data
        const char *tc_path = testCase[i].arg[0].sptr;
        char **tc_filter = testCase[i].arg[1].slptr;
        unsigned int tc_mode = testCase[i].arg[2].unsigned_int;

        // Set up temporary test directory name
        sprintf(tmpdir, "fstree_test%zu_XXXXXX", i);
        // Create directory
        mkdtemp(tmpdir);

        // Enter the new temporary directory
        chdir(tmpdir); {
            // Populate the relative (or untouched) current working directory buffer
            getcwd(cwd, PATH_MAX);
            // Obtain the absolute path to the current working directory
            realpath(tc_path, tc_abspath);

            // Generate a mock file and rename it to hello_world.txt
            char *tmpfile = mock_size(1 * sizeof(char), "?");
            rename(tmpfile, hello_world);
            free(tmpfile);

            // --- Test begins here

            // Read-in directory structure
            FSTree *fsdata = fstree(tc_path, tc_filter, tc_mode);

            myassert(fsdata != NULL, "FSTree was NULL");
            myassert(fsdata->root, "FSTree root path was NULL");
            myassert(strcmp(tc_abspath, cwd) == 0, "current directory is '%s', but should have been '%s'\n", tc_abspath, cwd);
            myassert(fsdata->num_records > 0, "num_records should be non-zero\n");
            myassert(fsdata->_num_alloc > 0, "_num_alloc should be non-zero\n");
            for (size_t f = 0; f < fsdata->num_records; f++) {
                myassert(fsdata->record[f]->name != NULL, "FSRec name record was NULL");
                myassert(fsdata->record[f]->st != NULL, "FSRec stat struct was NULL");
            }

            fstree_free(fsdata);
        }
        // Return from temporary directory
        chdir(startdir);

        // Delete temporary directory
        rmdirs(tmpdir);
    }
}