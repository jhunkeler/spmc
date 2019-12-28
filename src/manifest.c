/**
 * @file manifest.c
 */
#include "spm.h"
#include <fnmatch.h>
#define PACKAGE_MIN_DELIM 2

/**
 * Generate a `Manifest` of package data
 * @param package_dir a directory containing SPM packages
 * @return `Manifest`
 */
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
            info->packages[i]->requirements_records = deps->records;
            int j;
            for (j = 0; j < deps->records; j++) {
                info->packages[i]->requirements[j] = (char *) calloc(strlen(deps->list[j]) + 1, sizeof(char));
                strncpy(info->packages[i]->requirements[j], deps->list[j], strlen(deps->list[j]));
            }
        }
        dep_free(&deps);

        // Replace unwanted hyphens in the package name with an invalid character to prevent splitting on the wrong
        // hyphen below
        int delims = num_chars(fsdata->files[i], '-');
        if (delims > PACKAGE_MIN_DELIM) {
            for (int t = strlen(fsdata->files[i]); t != 0; t--) {
                if (fsdata->files[i][t] == '-') {
                    delims--;
                    if (delims == 0) {
                        fsdata->files[i][t] = '*';
                    }
                }
            }
        }

        // Split the package name into parts (invalid characters are ignored)
        char **parts = split(fsdata->files[i], "-");

        // Replace invalid character with a hyphen
        replace_text(parts[0], "*", "-");
        replace_text(fsdata->files[i], "*", "-");

        // Populate `ManifestPackage` record
        info->packages[i]->size = get_file_size(fsdata->files[i]);
        strncpy(info->packages[i]->archive, basename(fsdata->files[i]), PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->name, basename(parts[0]), PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->version, parts[1], PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->revision, parts[2], PACKAGE_MEMBER_SIZE);
        strdelsuffix(info->packages[i]->revision, SPM_PACKAGE_EXTENSION);
        split_free(parts);
    }

    return info;
}

/**
 * Free a `Manifest` structure
 * @param info `Manifest`
 */
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

/**
 * Write a `Manifest` to the configuration directory
 * @param info
 * @return
 */
int manifest_write(Manifest *info) {
    const char *filename = "manifest.dat";
    char path[PATH_MAX];
    memset(path, '\0', sizeof(path));
    sprintf(path, "%s%c%s", SPM_GLOBAL.user_config_basedir, DIRSEP, filename);
    FILE *fp = fopen(path, "w+");
    char *reqs = NULL;

    // A little too much information (debug?)
    if (SPM_GLOBAL.verbose) {
        for (int i = 0; i < info->records; i++) {
            printf("%-20s: %s\n"
                   "%-20s: %lu\n"
                   "%-20s: %s\n"
                   "%-20s: %s\n"
                   "%-20s: %s\n"
                   "%-20s: %d\n",
                   "archive", info->packages[i]->archive,
                   "size", info->packages[i]->size,
                   "name", info->packages[i]->name,
                   "version", info->packages[i]->version,
                   "revision", info->packages[i]->revision,
                   "requirements_records", info->packages[i]->requirements_records
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
                      "%s|"  // name
                      "%s|"  // version
                      "%s|"  // revision
                      "%d|"  // requirements_records
                      "%s"   // requirements
                      , info->packages[i]->archive,
                      info->packages[i]->size,
                      info->packages[i]->name,
                      info->packages[i]->version,
                      info->packages[i]->revision,
                      info->packages[i]->requirements_records,
                      reqs ? reqs : "*");
        fprintf(fp, "%s\n", dptr);
        free(reqs);
    }
    fclose(fp);
    return 0;
}

/**
 * Read the package manifest stored in the configuration directory
 * @return `Manifest` structure
 */
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

    // Begin parsing the manifest
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
        info->packages[i]->requirements_records = atoi(parts[5]);

        info->packages[i]->requirements = NULL;
        if (strncmp(parts[6], "*", 2) != 0) {
            info->packages[i]->requirements = split(parts[6], ",");

        }
        split_free(parts);
        info->records = i;
        i++;
    }

    fclose(fp);
    return info;
}

/**
 * Find a package in a `Manifest`
 * @param info `Manifest`
 * @param _package package name
 * @return found=`ManifestPackage`, not found=NULL
 */
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