#include "spm.h"

int build(int argc, char **argv) {
    printf("build:\n");
    printf("argc: %d\n", argc);
    printf("argv:\n");
    for (int i = 0; i < argc; i++) {
        printf("%d: %s\n", i, argv[i]);
    }

    return 0;
}


