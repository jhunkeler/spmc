/**
 * @file manifest.c
 */
#include "spm.h"
#include <fnmatch.h>
#include "url.h"

/**
 * Compare `ManifestPackage` packages (lazily)
 * @param a
 * @param b
 * @return 0 = same, !0 = different
 */
int manifest_package_cmp(const ManifestPackage *a, const ManifestPackage *b) {
    int result = 0;
    if (a == NULL || b == NULL) {
        return -1;
    }
    result += strcmp(a->origin, b->origin);
    result += strcmp(a->archive, b->archive);
    result += strcmp(a->checksum_sha256, b->checksum_sha256);
    return result;
}

void manifest_package_separator_swap(char **name) {
    // Replace unwanted separators in the package name with placeholder to prevent splitting on the wrong one
    int delim_count = num_chars((*name), SPM_PACKAGE_MEMBER_SEPARATOR) - SPM_PACKAGE_MIN_DELIM;

    if (delim_count < 0) {
        return;
    }

    for (size_t t = 0; t < strlen((*name)); t++) {
        if (delim_count == 0) break;
        if ((*name)[t] == SPM_PACKAGE_MEMBER_SEPARATOR) {
            (*name)[t] = SPM_PACKAGE_MEMBER_SEPARATOR_PLACEHOLD;
            delim_count--;
        }
    }
}

void manifest_package_separator_restore(char **name) {
    char separator[2];
    char placeholder[2];
    snprintf(separator, sizeof(separator), "%c", SPM_PACKAGE_MEMBER_SEPARATOR);
    snprintf(placeholder, sizeof(placeholder), "%c", SPM_PACKAGE_MEMBER_SEPARATOR_PLACEHOLD);

    replace_text((*name), placeholder, separator);
}

/**
 * Generate a `Manifest` of package data
 * @param package_dir a directory containing SPM packages
 * @return `Manifest`
 */
Manifest *manifest_from(const char *package_dir) {
    char *package_filter[] = {SPM_PACKAGE_EXTENSION, NULL}; // We only want packages
    FSTree *fsdata = NULL;
    fsdata = fstree(package_dir, package_filter, SPM_FSTREE_FLT_ENDSWITH);

    Manifest *info = (Manifest *)calloc(1, sizeof(Manifest));
    info->records = fsdata->files_length;
    info->packages = (ManifestPackage **) calloc(info->records + 1, sizeof(ManifestPackage *));
    if (info->packages == NULL) {
        perror("Failed to allocate package array");
        fprintf(SYSERROR);
        free(info);
        fstree_free(fsdata);
        return NULL;
    }

    if (SPM_GLOBAL.verbose) {
        printf("Initializing package manifest:\n");
    }
    strncpy(info->origin, package_dir, SPM_PACKAGE_MEMBER_ORIGIN_SIZE);


    char *tmpdir = spm_mkdtemp(TMP_DIR, "spm_manifest_from", NULL);
    if (!tmpdir) {
        perror("failed to create temporary directory");
        fprintf(SYSERROR);
        free(info);
        fstree_free(fsdata);
        return NULL;
    }

    for (size_t i = 0; i < fsdata->files_length; i++) {
        float percent = (((float)i + 1) / fsdata->files_length) * 100;

        if (SPM_GLOBAL.verbose) {
            printf("[%3.0f%%] %s\n", percent, basename(fsdata->files[i]));
        }

        // Initialize package record
        info->packages[i] = (ManifestPackage *) calloc(1, sizeof(ManifestPackage));
        if (info->packages[i] == NULL) {
            perror("Failed to allocate package record");
            fprintf(SYSERROR);
            fstree_free(fsdata);
            free(info);
            rmdirs(tmpdir);
            return NULL;
        }

        // Swap extra package separators with a bogus character
        manifest_package_separator_swap(&fsdata->files[i]);

        // Split the package name into parts
        char psep[2];
        snprintf(psep, sizeof(psep), "%c", SPM_PACKAGE_MEMBER_SEPARATOR);
        char **parts = split(fsdata->files[i], psep);

        // Restore package separator
        manifest_package_separator_restore(&parts[0]);
        manifest_package_separator_restore(&fsdata->files[i]);

        // Populate `ManifestPackage` record
        info->packages[i]->size = (size_t) get_file_size(fsdata->files[i]);
        strncpy(info->packages[i]->origin, info->origin, SPM_PACKAGE_MEMBER_ORIGIN_SIZE);
        strncpy(info->packages[i]->archive, basename(fsdata->files[i]), SPM_PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->name, basename(parts[0]), SPM_PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->version, parts[1], SPM_PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->revision, parts[2], SPM_PACKAGE_MEMBER_SIZE);
        strdelsuffix(info->packages[i]->revision, SPM_PACKAGE_EXTENSION);

        // Read package requirement specs
        char *archive = join((char *[]) {info->origin, info->packages[i]->archive, NULL}, DIRSEPS);
        if (tar_extract_file(archive, SPM_META_DEPENDS, tmpdir) != 0) {
            // TODO: at this point is the package is invalid? .SPM_DEPENDS should be there...
            fprintf(stderr, "extraction failure: %s\n", archive);
            rmdirs(tmpdir);
            exit(1);
        }
        char *depfile = join((char *[]) {tmpdir, SPM_META_DEPENDS, NULL}, DIRSEPS);
        info->packages[i]->requirements = file_readlines(depfile, 0, 0, NULL);

        // Record count of requirement specs
        if (info->packages[i]->requirements != NULL) {
            for (size_t rec = 0; info->packages[i]->requirements[rec] != NULL; rec++) {
                strip(info->packages[i]->requirements[rec]);
                info->packages[i]->requirements_records++;
            }
        }

        unlink(depfile);
        free(depfile);
        free(archive);
        split_free(parts);
    }

    fstree_free(fsdata);
    rmdirs(tmpdir);
    return info;
}

/**
 * Free a `Manifest` structure
 * @param info `Manifest`
 */
void manifest_free(Manifest *info) {
    for (size_t i = 0; i < info->records; i++) {
        if (info->packages[i] != NULL) {
            manifest_package_free(info->packages[i]);
        }
    }
    free(info);
}

/**
 * Free a `ManifestPackage` structure
 * @param info `ManifestPackage`
 */
void manifest_package_free(ManifestPackage *info) {
    if (info->requirements == NULL) {
        return;
    }

    for (size_t i = 0; i < info->requirements_records; i++) {
        if (info->requirements[i] != NULL) {
            free(info->requirements[i]);
        }
    }
    free(info->requirements);
    free(info);
}

/**
 * Write a `Manifest` to the configuration directory
 * @param info
 * @param pkgdir
 * @return
 */
int manifest_write(Manifest *info, const char *pkgdir) {
    char *reqs = NULL;
    char path[PATH_MAX];
    char path_manifest[PATH_MAX];

    memset(path, '\0', sizeof(path));
    memset(path_manifest, '\0', sizeof(path));

    strcpy(path, pkgdir);

    // Append the repo target if its missing
    if (strstr(path, SPM_GLOBAL.repo_target) == NULL) {
        strcat(path, DIRSEPS);
        strcat(path, SPM_GLOBAL.repo_target);
    }
    strcpy(path_manifest, path);

    // Append the manifest filename if its missing
    if (!endswith(path_manifest, SPM_MANIFEST_FILENAME)) {
        strcat(path_manifest, DIRSEPS);
        strcat(path_manifest, SPM_MANIFEST_FILENAME);
    }

    FILE *fp = fopen(path_manifest, "w+");
    if (fp == NULL) {
        perror(path_manifest);
        fprintf(SYSERROR);
        return -1;
    }
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

    if (SPM_GLOBAL.verbose) {
        printf("Generating manifest file: %s\n", path_manifest);
    }
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
        char *archive = join((char *[]) {path, info->packages[i]->archive, NULL}, DIRSEPS);
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
        free(archive);
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
        fprintf(stderr, "couldn't url_fopen() %s\n", url);
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
    char *tmpdir = NULL;
    char path[PATH_MAX];
    char *pathptr = path;
    memset(path, '\0', PATH_MAX);

    // When file_or_url is NULL we want to use the global manifest
    if (file_or_url == NULL) {
        // TODO: move this out
        strcpy(path, SPM_GLOBAL.package_dir);
    }
    else {
        tmpdir = spm_mkdtemp(TMP_DIR, "spm_manifest_read_XXXXXX", SPM_GLOBAL.repo_target);
        if (exists(tmpdir) != 0) {
            fprintf(stderr, "Failed to create temporary storage directory\n");
            fprintf(SYSERROR);
            return NULL;
        }

        snprintf(pathptr, PATH_MAX - 1, "%s%s%s%s%s", tmpdir, DIRSEPS, SPM_GLOBAL.repo_target, DIRSEPS, filename);
    }

    const char *target_is;
    if (strstr(file_or_url, SPM_GLOBAL.repo_target) != NULL) {
        target_is = "";
    }
    else {
        target_is = SPM_GLOBAL.repo_target;
    }

    char *remote_manifest = join_ex(DIRSEPS, file_or_url, target_is, filename, NULL);

    if (exists(pathptr) != 0) {
        // TODO: Move this out
        int fetch_status = fetch(remote_manifest, pathptr);
        if (fetch_status >= 400) {
            fprintf(stderr, "HTTP %d: %s: %s\n", fetch_status, http_response_str(fetch_status), remote_manifest);
            free(remote_manifest);
            return NULL;
        }
        else if (fetch_status == 1 || fetch_status < 0) {
            free(remote_manifest);
            return NULL;
        }
    }

    int valid = 0;
    size_t total_records = 0;
    char data[BUFSIZ];
    char *dptr = data;
    memset(dptr, '\0', BUFSIZ);

    fp = fopen(pathptr, "r+");
    if (!fp) {
        perror(filename);
        fprintf(SYSERROR);
        return NULL;
    }

    while (fgets(dptr, BUFSIZ, fp) != NULL) {
        total_records++;
    }
    total_records--; // header does not count
    rewind(fp);

    Manifest *info = (Manifest *)calloc(1, sizeof(Manifest));
    info->packages = (ManifestPackage **)calloc(total_records + 1, sizeof(ManifestPackage *));

    // Record manifest's origin
    memset(info->origin, '\0', SPM_PACKAGE_MEMBER_ORIGIN_SIZE);
    if (remote_manifest != NULL) {
        strcpy(info->origin, remote_manifest);
    }
    else {
        strcpy(info->origin, path);
    }
    free(remote_manifest);

    // Check validity of the manifest's formatting and field length
    if ((valid = manifest_validate()) != 0) {
        return NULL;
    }

    // Begin parsing the manifest
    const char separator[] = {SPM_MANIFEST_SEPARATOR, '\0'};
    size_t i = 0;

    // Consume header
    if (fgets(dptr, BUFSIZ, fp) == NULL) {
        // file is probably empty
        return NULL;
    }

    info->records = total_records;
    while (fgets(dptr, BUFSIZ, fp) != NULL) {
        dptr = strip(dptr);
        char *garbage = NULL;
        char **parts = split(dptr, separator);
        char *_origin = NULL;

        if (file_or_url != NULL) {
            _origin = strdup(file_or_url);
        }
        else {
            _origin = dirname(path);
        }

        info->packages[i] = (ManifestPackage *)calloc(1, sizeof(ManifestPackage));

        strncpy(info->packages[i]->origin, _origin, SPM_PACKAGE_MEMBER_ORIGIN_SIZE);
        free(_origin);

        strncpy(info->packages[i]->archive, parts[0], SPM_PACKAGE_MEMBER_SIZE);
        info->packages[i]->size = strtoul(parts[1], &garbage, 10);
        strncpy(info->packages[i]->name, parts[2], SPM_PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->version, parts[3], SPM_PACKAGE_MEMBER_SIZE);
        strncpy(info->packages[i]->revision, parts[4], SPM_PACKAGE_MEMBER_SIZE);
        info->packages[i]->requirements_records = (size_t) atoi(parts[5]);

        info->packages[i]->requirements = NULL;
        if (strncmp(parts[6], SPM_MANIFEST_NODATA, strlen(SPM_MANIFEST_NODATA)) != 0) {
            info->packages[i]->requirements = split(parts[6], ",");
        }
        if (strncmp(parts[7], SPM_MANIFEST_NODATA, strlen(SPM_MANIFEST_NODATA)) != 0) {
            memset(info->packages[i]->checksum_sha256, '\0', SHA256_DIGEST_STRING_LENGTH);
            strcpy(info->packages[i]->checksum_sha256, parts[7]);

        }

        split_free(parts);
        i++;
    }

    if (tmpdir != NULL) {
        rmdirs(tmpdir);
        free(tmpdir);
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
ManifestPackage *manifest_search(const Manifest *info, const char *_package) {
    ManifestPackage *match = NULL;
    char *package = strdup(_package);

    if ((match = find_by_strspec(info, package)) != NULL) {
        free(package);
        return match;
    }

    free(package);
    return NULL;
}

/**
 * Duplicate a `ManifestPackage`
 * @param manifest
 * @return `ManifestPackage`
 */
ManifestPackage *manifest_package_copy(const ManifestPackage *manifest) {
    if (manifest == NULL) {
        return NULL;
    }

    ManifestPackage *result = calloc(1, sizeof(ManifestPackage));
    memcpy(result, manifest, sizeof(ManifestPackage));

    if (manifest->requirements_records > 0 && manifest->requirements != NULL) {
        result->requirements = (char **)calloc(manifest->requirements_records, sizeof(char *));
        for (size_t i = 0; i < manifest->requirements_records; i++) {
            result->requirements[i] = strdup(manifest->requirements[i]);
        }
    }

    return result;
}

/**
 *
 * @param ManifestList `pManifestList`
 */
void manifestlist_free(ManifestList *pManifestList) {
    for (size_t i = 0; i < pManifestList->num_inuse; i++) {
        if (pManifestList->data[i] != NULL) {
            manifest_free(pManifestList->data[i]);
        }
    }
    free(pManifestList->data);
    free(pManifestList);
}

/**
 * Append a value to the list
 * @param ManifestList `pManifestList`
 * @param str
 */
void manifestlist_append(ManifestList *pManifestList, char *path) {
    Manifest *manifest = manifest_read(path);
    if (manifest == NULL) {
        fprintf(stderr, "Failed to create manifest in memory\n");
        fprintf(SYSERROR);
        exit(1);
    }

    Manifest **tmp = realloc(pManifestList->data, (pManifestList->num_alloc + 1) * sizeof(Manifest *));
    if (tmp == NULL) {
        manifestlist_free(pManifestList);
        perror("failed to append to array");
        exit(1);
    }
    pManifestList->data = tmp;
    pManifestList->data[pManifestList->num_inuse] = manifest;
    pManifestList->num_inuse++;
    pManifestList->num_alloc++;
}

ManifestPackage *manifestlist_search(ManifestList *pManifestList, const char *_package) {
    ManifestPackage *found[255] = {0,};
    ManifestPackage *result = NULL;
    ssize_t offset = -1;

    for (size_t i = 0; i < manifestlist_count(pManifestList); i++) {
        Manifest *item = manifestlist_item(pManifestList, i);
        result = manifest_search(item, _package);
        if (result != NULL) {
            offset++;
            found[offset] = result;
        }
    }

    if (offset < 0) {
        return NULL;
    }
    return found[offset];
}

/**
 * Get the count of values stored in a `pManifestList`
 * @param ManifestList
 * @return
 */
size_t manifestlist_count(const ManifestList *pManifestList) {
    return pManifestList->num_inuse;
}

/**
 * Set value at index
 * @param ManifestList
 * @param value string
 * @return
 */
void manifestlist_set(ManifestList *pManifestList, size_t index, Manifest *value) {
    Manifest *item = NULL;
    if (index > manifestlist_count(pManifestList)) {
        return;
    }
    if ((item = manifestlist_item(pManifestList, index)) == NULL) {
        return;
    }
    memcpy(pManifestList->data[index], value, sizeof(Manifest));
}

/**
 * Retrieve data from a `pManifestList`
 * @param ManifestList
 * @param index
 * @return string
 */
Manifest *manifestlist_item(const ManifestList *pManifestList, size_t index) {
    if (index > manifestlist_count(pManifestList)) {
        return NULL;
    }
    return pManifestList->data[index];
}

/**
 * Initialize an empty `pManifestList`
 * @return `pManifestList`
 */
ManifestList *manifestlist_init() {
    ManifestList *pManifestList = calloc(1, sizeof(ManifestList));
    if (pManifestList == NULL) {
        perror("failed to allocate array");
        exit(errno);
    }
    pManifestList->num_inuse = 0;
    pManifestList->num_alloc = 1;
    pManifestList->data = calloc(pManifestList->num_alloc, sizeof(char *));
    return pManifestList;
}


