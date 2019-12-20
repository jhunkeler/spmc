//
// Created by jhunk on 12/20/19.
//
#include "spm.h"

int manifest_create(const char *package_dir) {
    const char *filename = "manifest.dat";
    char path[PATH_MAX];
    int total_records = 0;
    FSTree *fsdata = NULL;
    fsdata = fstree(package_dir);
    ManifestPackage **info = (ManifestPackage **)calloc(fsdata->files_length + 1, sizeof(ManifestPackage *));

    memset(path, '\0', sizeof(path));
    memset(info, '\0', sizeof(ManifestPackage *));
    sprintf(path, "%s%c%s", SPM_GLOBAL.user_config_basedir, DIRSEP, filename);

    printf("Gathering package data:\n");
    for (int i = 0; i < fsdata->files_length && i < 0xffff; i++) {
        float percent = (((float)i + 1) / fsdata->files_length) * 100;
        printf("[%3.0f%%] %s\n", percent, basename(fsdata->files[i]));
        Dependencies *deps = NULL;
        dep_init(&deps);
        if (dep_all(&deps, basename(fsdata->files[i])) < 0) {
            dep_free(&deps);
        }

        // Initialize package record
        info[i] = (ManifestPackage *)calloc(1, sizeof(ManifestPackage));

        // Copy dependencies
        if (deps->records) {
            info[i]->requirements = (char **) calloc(deps->__size, sizeof(char *));
            for (int j = 0; j < deps->records; j++) {
                info[i]->requirements[j] = (char *) calloc(strlen(deps->list[j]) + 1, sizeof(char));
                strncpy(info[i]->requirements[j], deps->list[j], strlen(deps->list[j]));
            }
        }
        dep_free(&deps);

        char **parts = split(fsdata->files[i], "-");
        info[i]->size = get_file_size(fsdata->files[i]);
        strncpy(info[i]->archive, basename(fsdata->files[i]), PACKAGE_MEMBER_SIZE);
        strncpy(info[i]->name, basename(parts[0]), PACKAGE_MEMBER_SIZE);
        strncpy(info[i]->version, parts[1], PACKAGE_MEMBER_SIZE);
        strncpy(info[i]->revision, parts[2], PACKAGE_MEMBER_SIZE);
        strdelsuffix(info[i]->revision, SPM_PACKAGE_EXTENSION);
        split_free(parts);
        total_records++;
    }
    printf("\n");

    FILE *fp = fopen(path, "w+");
    char *reqs;
    for (int i = 0; i < total_records; i++) {
        if (SPM_GLOBAL.verbose) {
            printf("%-20s: %s\n"
                   "%-20s: %lu\n"
                   "%-20s: %s\n"
                   "%-20s: %s\n"
                   "%-20s: %s\n",
                   "archive", info[i]->archive,
                   "size", info[i]->size,
                   "name", info[i]->name,
                   "version", info[i]->version,
                   "revision", info[i]->revision
            );
            reqs = join(info[i]->requirements, ", ");
            printf("%-20s: %s\n", "requirements", reqs ? reqs : "NONE");
            free(reqs);
            printf("\n");
        }

    }

    printf("Generating manifest: %s\n", path);
    for (int i = 0; i < total_records; i++) {
        // write CSV-like manifest
        char data[BUFSIZ];
        memset(data, '\0', BUFSIZ);
        char *dptr = data;
        float percent = (((float)i + 1) / total_records) * 100;
        printf("[%3.0f%%] %s\n", percent, info[i]->archive);
        reqs = join(info[i]->requirements, ",");
        sprintf(dptr, "%s|" // archive
                      "%lu|" // size
                      "%s|" // name
                      "%s|" // version
                      "%s|" // revision
                      "%s" // requirements
                      , info[i]->archive, info[i]->size, info[i]->name, info[i]->version, info[i]->revision, reqs ? reqs : "*");
        fprintf(fp, "%s\n", dptr);
        free(reqs);
    }
    fclose(fp);
}
