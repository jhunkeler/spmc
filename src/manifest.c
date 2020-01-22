/**
 * @file manifest.c
 */
#include "spm.h"
#include <fnmatch.h>
#include "url.h"
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
    for (size_t i = 0; i < fsdata->files_length; i++) {
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
            size_t j;
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
            for (size_t t = strlen(fsdata->files[i]); t != 0; t--) {
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
        replace_text(parts[0], SPM_MANIFEST_NODATA, "-");
        replace_text(fsdata->files[i], SPM_MANIFEST_NODATA, "-");

        // Populate `ManifestPackage` record
        info->packages[i]->size = (size_t) get_file_size(fsdata->files[i]);
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
    for (size_t i = 0; i < info->records; i++) {
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
    char *reqs = NULL;
    char path[PATH_MAX];
    memset(path, '\0', sizeof(path));
    strcpy(path, SPM_GLOBAL.package_manifest);

    FILE *fp = fopen(path, "w+");
#ifdef _DEBUG
    if (SPM_GLOBAL.verbose) {
        for (size_t i = 0; i < info->records; i++) {
            printf("%-20s: %s\n"
                   "%-20s: %zu\n"
                   "%-20s: %s\n"
                   "%-20s: %s\n"
                   "%-20s: %s\n"
                   "%-20s: %zu\n",
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
#endif

    printf("Generating manifest file: %s\n", path);
    fprintf(fp, "%s\n", SPM_MANIFEST_HEADER);
    char data[BUFSIZ];
    for (size_t i = 0; i < info->records; i++) {
        // write CSV-like manifest
        memset(data, '\0', BUFSIZ);
        char *dptr = data;
        float percent = (((float)i + 1) / info->records) * 100;
        if (SPM_GLOBAL.verbose) {
            printf("[%3.0f%%] %s\n", percent, info->packages[i]->archive);
        }
        reqs = join(info->packages[i]->requirements, ",");
        char *archive = join((char *[]) {SPM_GLOBAL.package_dir, info->packages[i]->archive, NULL}, DIRSEPS);
        char *checksum_sha256 = sha256sum(archive);

        sprintf(dptr, "%s|" // archive
                      "%zu|" // size
                      "%s|"  // name
                      "%s|"  // version
                      "%s|"  // revision
                      "%zu|"  // requirements_records
                      "%s|"   // requirements
                      "%s"   // checksum_md5
                      , info->packages[i]->archive,
                      info->packages[i]->size,
                      info->packages[i]->name,
                      info->packages[i]->version,
                      info->packages[i]->revision,
                      info->packages[i]->requirements_records,
                      reqs ? reqs : SPM_MANIFEST_NODATA,
                      checksum_sha256 ? checksum_sha256 : SPM_MANIFEST_NODATA);
                fprintf(fp, "%s\n", dptr);
        free(reqs);
        if (checksum_sha256 != NULL)
            free(checksum_sha256);
    }
    fclose(fp);
    return 0;
}

/**
 *
 * @param url
 * @param dest
 * @return
 */
int fetch(const char *url, const char *dest) {
    URL_FILE *handle = NULL;
    FILE *outf = NULL;
    size_t chunk_size = 0xffff;
    size_t nread = 0;
    char *buffer = calloc(chunk_size + 1, sizeof(char));
    if (!buffer) {
        perror("fetch buffer too big");
        return -1;
    }

    handle = url_fopen(url, "r");
    if(!handle) {
        printf("couldn't url_fopen() %s\n", url);
        return 2;
    }

    outf = fopen(dest, "wb+");
    if(!outf) {
        perror("couldn't open fread output file\n");
        return 1;
    }

    do {
        nread = url_fread(buffer, 1, chunk_size, handle);
        if (handle->http_status >= 400) {
            free(buffer);
            fclose(outf);
            if (exists(dest) == 0) {
                unlink(dest);
            }

            long http_status = handle->http_status;
            url_fclose(handle);
            return http_status;
        }
        fwrite(buffer, 1, nread, outf);
    } while (nread);

    free(buffer);
    fclose(outf);
    url_fclose(handle);
    return 0;
}

int manifest_validate(void) {
    size_t line_count;
    int problems;
    char data[BUFSIZ];
    FILE *fp;

    if (exists(SPM_GLOBAL.package_manifest) != 0) {
        return -1;
    }

    if ((fp = fopen(SPM_GLOBAL.package_manifest, "r")) == NULL) {
        perror(SPM_GLOBAL.package_manifest);
        return -2;
    }

    line_count = 0;
    problems = 0;
    while (fgets(data, BUFSIZ, fp) != NULL) {
        int separators;
        if (line_count == 0) {
            if (strncmp(data, SPM_MANIFEST_HEADER, strlen(SPM_MANIFEST_HEADER)) != 0) {
                fprintf(stderr, "Invalid manifest header: %s (expecting '%s')\n", strip(data), SPM_MANIFEST_HEADER);
                problems++;
                line_count++;
            }
        }
        else if ((separators = num_chars(data, SPM_MANIFEST_SEPARATOR)) != SPM_MANIFEST_SEPARATOR_MAX) {
            fprintf(stderr, "Invalid manifest record on line %zu: %s (expecting %d separators, found %d)\n", line_count, strip(data), SPM_MANIFEST_SEPARATOR_MAX, separators);
            problems++;
        }
        line_count++;
    }
    return problems;
}
/**
 * Read the package manifest stored in the configuration directory
 * @return `Manifest` structure
 */
Manifest *manifest_read(char *file_or_url) {
    FILE *fp = NULL;
    char *filename = SPM_MANIFEST_FILENAME;
    char path[PATH_MAX];

    // When file_or_url is NULL we want to use the global manifest
    if (file_or_url == NULL) {
        // TODO: move this out
        strcpy(path, SPM_GLOBAL.package_manifest);
    }
    else {
        strcpy(path, file_or_url);
    }

    // Handle receiving a path without the manifest filename
    // by appending the manifest to the path
    if (endswith(path, filename) != 0) {
        strcat(path, DIRSEPS);
        strcat(path, filename);
    }

    if (exists(path) != 0) {
        // TODO: Move this out
        char *remote_manifest = join((char *[]) {"http://astroconda.org/spm", SPM_GLOBAL.repo_target, filename, NULL}, DIRSEPS);
        int fetch_status = fetch(remote_manifest, path);
        if (fetch_status >= 400) {
            fprintf(stderr, "HTTP %d: %s: %s\n", fetch_status, http_response_str(fetch_status), remote_manifest);
            free(remote_manifest);
            return NULL;
        }
        free(remote_manifest);
    }

    int valid = 0;
    size_t total_records = 0;
    char data[BUFSIZ];
    char *dptr = data;
    memset(dptr, '\0', BUFSIZ);

    fp = fopen(path, "r+");
    if (!fp) {
        perror(filename);
        fprintf(SYSERROR);
        return NULL;
    }

    while (fgets(dptr, BUFSIZ, fp) != NULL) {
        total_records++;
    }
    rewind(fp);

    Manifest *info = (Manifest *)calloc(1, sizeof(Manifest));
    info->packages = (ManifestPackage **)calloc(total_records + 1, sizeof(ManifestPackage *));

    if ((valid = manifest_validate()) != 0) {
        return NULL;
    }

    // Begin parsing the manifest
    char separator = SPM_MANIFEST_SEPARATOR;
    size_t i = 0;

    // Consume header
    if (fgets(dptr, BUFSIZ, fp) == NULL) {
        // file is probably empty
        return NULL;
    }

    while (fgets(dptr, BUFSIZ, fp) != NULL) {
        dptr = strip(dptr);
        char *garbage;
        char **parts = split(dptr, &separator);
        char *_origin = dirname(path);

        info->packages[i] = (ManifestPackage *)calloc(1, sizeof(ManifestPackage));

        strncpy(info->packages[i]->origin, _origin, strlen(_origin));
        free(_origin);

        strncpy(info->packages[i]->archive, parts[0], strlen(parts[0]));
        info->packages[i]->size = strtoul(parts[1], &garbage, 10);
        strncpy(info->packages[i]->name, parts[2], strlen(parts[2]));
        strncpy(info->packages[i]->version, parts[3], strlen(parts[3]));
        strncpy(info->packages[i]->revision, parts[4], strlen(parts[4]));
        info->packages[i]->requirements_records = (size_t) atoi(parts[5]);

        info->packages[i]->requirements = NULL;
        if (strncmp(parts[6], SPM_MANIFEST_NODATA, strlen(SPM_MANIFEST_NODATA)) != 0) {
            info->packages[i]->requirements = split(parts[6], ",");

        }
        if (strncmp(parts[7], SPM_MANIFEST_NODATA, strlen(SPM_MANIFEST_NODATA)) != 0) {
            strncpy(info->packages[i]->checksum_sha256, parts[7], strlen(parts[7]));
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

    for (size_t i = 0; i < info->records; i++) {
        if (fnmatch(package, info->packages[i]->archive, FNM_PATHNAME) == 0) {
            return info->packages[i];
        }
    }
    return NULL;
}