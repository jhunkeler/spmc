#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/**
 * Apple loves being different. There's no `realpath` program on MacOS
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    char *path;

    if (argc < 2) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        return 1;
    }

    path = realpath(argv[1], NULL);
    if (path) {
        puts(path);
        free(path);
    } else {
        perror(argv[1]);
    }

    return errno;
}