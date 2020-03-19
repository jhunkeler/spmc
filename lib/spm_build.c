/**
 * @file spm_build.c
 */
#include "spm.h"

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int build(int argc, char **argv) {
    printf("build:\n");
    printf("argc: %d\n", argc);
    printf("argv:\n");
    for (int i = 0; i < argc; i++) {
        printf("%d: %s\n", i, argv[i]);
    }

    return 0;
}


