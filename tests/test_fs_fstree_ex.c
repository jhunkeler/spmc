#include "spm.h"
#include "framework.h"

const char *testFmt = "returned '%s', expected '%s'\n";
struct TestCase testCase[] = {
        {},
};
size_t numCases = sizeof(testCase) / sizeof(struct TestCase);

static int sortby_inode(const void *a, const void *b) {
    FSRec **ia = (FSRec **)a;
    FSRec **ib = (FSRec **)b;
    return (*ia)->st->st_ino > (*ib)->st->st_ino;
}

int main(int argc, char *argv[]) {
    for (size_t i = 0; i < numCases; i++) {
        // myassert()
    }


    FSTree *fsdata = fstree(".", (char *[]){"", NULL}, SPM_FSTREE_FLT_ENDSWITH);
    if (fsdata == NULL) {
        spm_perror("fstree");
        exit(1);
    }
    qsort(fsdata->record, fsdata->num_records, sizeof(FSRec *), sortby_inode);

    printf("root: %s\n", fsdata->root);
    /*
    for (size_t i = 0; i < fsdata->num_records; i++) {
        if (S_ISDIR(fsdata->record[i]->st->st_mode) != 0) {
            printf("dir: %s: %lu\n", fsdata->record[i]->name, fsdata->record[i]->st->st_ino);
        }
    }
    for (size_t i = 0; i < fsdata->num_records; i++) {
        if (S_ISREG(fsdata->record[i]->st->st_mode) != 0) {
            printf("file: %s: %lu: %lu\n", fsdata->record[i]->name, fsdata->record[i]->st->st_ino, fsdata->record[i]->st->st_nlink);
        }
    }
     */
    StrList *list = strlist_init();

    for (size_t i = 0; i < fsdata->num_records; i++) {
        FSRec *current = fsdata->record[i];

        if (!S_ISREG(current->st->st_mode)) {
            continue;
        }

        if (current->st->st_nlink < 2) {
            continue;
        }

        char value[PATH_MAX];
        sprintf(value, "%lu,%lu,%s", current->st->st_ino, current->st->st_nlink, current->name);
        strlist_append(list, value);
    }

    for (size_t i = 0; i < strlist_count(list); i++) {
        char *item = strlist_item(list, i);
        printf("item[%zu]: %s\n", i, item);
    }

    fstree_free(fsdata);
    strlist_free(list);
    return 0;
}