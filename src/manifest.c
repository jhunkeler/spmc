//
// Created by jhunk on 12/20/19.
//
#include "spm.h"
#include <fnmatch.h>

Manifest *manifest_from(const char *package_dir) {
    FSTree *fsdata = NULL;
    fsdata = fstree(package_dir);
    Manifest *info = (Manifest *)calloc(1, sizeof(Manifest));
    info->records = fsdata->files_length;
    info->packages = (ManifestPackage **) calloc(info->records + 1, sizeof(ManifestPackage *));

    printf("Initializing package manifest:\n");
    for (int i = 0; i < fsdata->files_length; i++) {
        float percent = (((float)i + 1) / fsdata->files_length) * 100;
        printf("[%3.0f%%] %s\n", percent, basename(fsdata->files[i]));
        Dependencies *deps = NULL;
        dep_init(&deps);
        if (dep_all(&deps, basename(fsdata->files[i])) < 0) {
            dep_free(&deps);
        }

        // Initialize package record
        info->packages[i] = (ManifestPackage *) calloc(1, sizeof(ManifestPackage));

        // Copy dependencies
        if (deps->records) {
            info->packages[i]->requirements = (char **) calloc(deps->__size, sizeof(char *));
            for (int j = 0; j < deps->records; j++) {
                info->packages[i]->requirements[j] = (char *) calloc(strlen(deps->list[j]) + 1, sizeof(char));
                strncpy(info->packages[i]->requirements[j], deps->list[j], strlen(deps->list[j]));
            }
        }
        dep_free(&deps);

        char **parts = split(fsdata->files[i], "-");
        info->packages[i]->size = get_file_size(fsdata->files[i]);
        strncpy(info->packages[i]->archive, basename(fsdata->files[i]), PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->name, basename(parts[0]), PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->version, parts[1], PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->revision, parts[2], PACKAGE_MEMBER_SIZE);
        strdelsuffix(info->packages[i]->revision, SPM_PACKAGE_EXTENSION);
        split_free(parts);
    }
    //printf("\n");
    return info;
}

void manifest_free(Manifest *info) {
    for (int i = 0; i < info->records; i++) {
        if (info->packages[i]->requirements) {
            for (int j = 0; info->packages[i]->requirements[j] != NULL; j++) {
                free(info->packages[i]->requirements[j]);
            }
            free(info->packages[i]->requirements);
        }
        free(info->packages[i]);
    }
    free(info->packages);
    free(info);
}

int manifest_write(Manifest *info) {
    const char *filename = "manifest.dat";
    char path[PATH_MAX];
    memset(path, '\0', sizeof(path));
    sprintf(path, "%s%c%s", SPM_GLOBAL.user_config_basedir, DIRSEP, filename);
    FILE *fp = fopen(path, "w+");
    char *reqs = NULL;

    if (SPM_GLOBAL.verbose) {
        for (int i = 0; i < info->records; i++) {
            printf("%-20s: %s\n"
                   "%-20s: %lu\n"
                   "%-20s: %s\n"
                   "%-20s: %s\n"
                   "%-20s: %s\n",
                   "archive", info->packages[i]->archive,
                   "size", info->packages[i]->size,
                   "name", info->packages[i]->name,
                   "version", info->packages[i]->version,
                   "revision", info->packages[i]->revision
            );
            reqs = join(info->packages[i]->requirements, ", ");
            printf("%-20s: %s\n", "requirements", reqs ? reqs : "NONE");
            free(reqs);
            printf("\n");
        }
    }

    printf("Generating manifest file: %s\n", path);
    for (int i = 0; i < info->records; i++) {
        // write CSV-like manifest
        char data[BUFSIZ];
        memset(data, '\0', BUFSIZ);
        char *dptr = data;
        float percent = (((float)i + 1) / info->records) * 100;
        printf("[%3.0f%%] %s\n", percent, info->packages[i]->archive);
        reqs = join(info->packages[i]->requirements, ",");
        sprintf(dptr, "%s|" // archive
                      "%lu|" // size
                      "%s|" // name
                      "%s|" // version
                      "%s|" // revision
                      "%s" // requirements
                      , info->packages[i]->archive, info->packages[i]->size, info->packages[i]->name, info->packages[i]->version, info->packages[i]->revision, reqs ? reqs : "*");
        fprintf(fp, "%s\n", dptr);
        free(reqs);
    }
    fclose(fp);
    return 0;
}

Manifest *manifest_read(void) {
    const char *filename = "manifest.dat";
    char path[PATH_MAX];
    memset(path, '\0', sizeof(path));
    sprintf(path, "%s%c%s", SPM_GLOBAL.user_config_basedir, DIRSEP, filename);
    FILE *fp = fopen(path, "r+");
    if (!fp) {
        perror(filename);
        return NULL;
    }
    int total_records = 0;
    char data[BUFSIZ];
    char *dptr = data;
    memset(dptr, '\0', BUFSIZ);

    while (fgets(dptr, BUFSIZ, fp) != NULL) {
        total_records++;
    }
    rewind(fp);

    Manifest *info = (Manifest *)calloc(1, sizeof(Manifest));
    info->packages = (ManifestPackage **)calloc(total_records + 1, sizeof(ManifestPackage *));

    int i = 0;
    while (fgets(dptr, BUFSIZ, fp) != NULL) {
        dptr = strip(dptr);
        char *garbage;
        char **parts = split(dptr, "|");

        info->packages[i] = (ManifestPackage *)calloc(1, sizeof(ManifestPackage));
        strncpy(info->packages[i]->archive, parts[0], strlen(parts[0]));
        info->packages[i]->size = strtoul(parts[1], &garbage, 10);
        strncpy(info->packages[i]->name, parts[2], strlen(parts[2]));
        strncpy(info->packages[i]->version, parts[3], strlen(parts[3]));
        strncpy(info->packages[i]->revision, parts[4], strlen(parts[4]));

        info->packages[i]->requirements = NULL;
        if (strncmp(parts[5], "*", 2) != 0) {
            info->packages[i]->requirements = split(parts[5], ",");
        }
        split_free(parts);
        info->records = i;
        i++;
    }

    fclose(fp);
    return info;
}

ManifestPackage *manifest_search(Manifest *info, const char *_package) {
    char package[PATH_MAX];

    memset(package, '\0', PATH_MAX);
    strncpy(package, _package, PATH_MAX);
    strcat(package, "*");

    for (int i = 0; i < info->records; i++) {
        if (fnmatch(package, info->packages[i]->archive, FNM_PATHNAME) == 0) {
            return info->packages[i];
        }
    }
    return NULL;
}