#include "spm.h"
#include "url.h"

char **file_readlines(const char *filename) {
    FILE *fp = NULL;
    char **result = NULL;
    char *buffer = NULL;
    size_t lines = 0;

    if ((fp = fopen(filename, "r")) == NULL) {
        perror(filename);
        fprintf(SYSERROR);
        return NULL;
    }

    // Allocate buffer
    if ((buffer = calloc(BUFSIZ + 1, sizeof(char))) == NULL) {
        perror("line buffer");
        fprintf(SYSERROR);
        fclose(fp);
        return NULL;
    }

    // count number the of lines in the file
    while ((fgets(buffer, BUFSIZ, fp)) != NULL) {
        lines++;
    }

    if (!lines) {
        free(buffer);
        fclose(fp);
        return NULL;
    }

    rewind(fp);

    // Populate results array
    result = calloc(lines + 1, sizeof(char *));
    for (size_t i = 0; i < lines; i++) {
        if (fgets(buffer, BUFSIZ, fp) == NULL) {
            break;
        }
        result[i] = strdup(buffer);
    }

    free(buffer);
    fclose(fp);
    return result;
}

char **mirror_list(const char *filename) {
    char **mirrors = file_readlines(filename);
    char **result = NULL;
    size_t count;
    for (count = 0; mirrors[count] != NULL; count++);

    if (!count) {
        return NULL;
    }

    result = calloc(count + 1, sizeof(char **));
    for (size_t i = 0; mirrors[i] != NULL; i++) {
        if (startswith(mirrors[i], "#") == 0 || isempty(mirrors[i])) {
            continue;
        }
        result[i] = join((char *[]) {mirrors[i], SPM_GLOBAL.repo_target, NULL}, DIRSEPS);
        free(mirrors[i]);
    }
    free(mirrors);
    return result;
}

void mirror_list_free(char **m) {
    if (m == NULL) {
        return;
    }
    for (size_t i = 0; m[i] != NULL; i++) {
        free(m[i]);
    }
    free(m);
}

void mirror_clone(Manifest *info, char *_dest) {
    char *dest = NULL;
    if (endswith(_dest, SPM_GLOBAL.repo_target) != 0) {
        dest = strdup(_dest);
    }
    else {
        dest = join((char *[]) {_dest, SPM_GLOBAL.repo_target, NULL}, DIRSEPS);
    }

    if (exists(dest) != 0 && mkdirs(dest, 0755) != 0) {
        perror("Unable to create mirror directory");
        fprintf(SYSERROR);
        exit(1);
    }

    printf("Remote: %s\n", info->origin);
    printf("Local: %s\n", dest);

    for (size_t i = 0; i < info->records; i++) {
        long response = 0;
        char *archive = join((char *[]) {info->packages[i]->origin, SPM_GLOBAL.repo_target, info->packages[i]->archive, NULL}, DIRSEPS);
        char *path = join((char *[]) {dest, info->packages[i]->archive, NULL}, DIRSEPS);
        if (exists(path) == 0) {
            char *checksum = sha256sum(path);
            if (strcmp(checksum, info->packages[i]->checksum_sha256) == 0) {
                printf("Skipped: %s\n", archive);
                free(checksum);
                free(archive);
                free(path);
                continue;
            }
        }
        printf("Fetch: %s\n", archive);
        if ((response = fetch(archive, path)) >= 400) {
            fprintf(stderr, "WARNING: HTTP(%ld, %s): %s\n", response, http_response_str(response), archive);
        }
        free(archive);
        free(path);
    }

    // Now fetch a copy of the physical manifest
    char *datafile = join((char *[]) {dest, basename(info->origin), NULL}, DIRSEPS);
    long response = 0;
    if ((response = fetch(info->origin, datafile) >= 400)) {
        fprintf(stderr, "WARNING: HTTP(%ld, %s): %s\n", response, http_response_str(response), info->origin);
    }
    free(dest);
    free(datafile);
    printf("done!\n");
}