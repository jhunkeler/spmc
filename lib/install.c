/**
 * @file install.c
 */
#include <url.h>
#include "spm.h"

/**
 * Check for the existence of `$ROOT/var/.spm_root`
 * @param path
 * @return yes=1, no=0
 */
int spm_hierarchy_is_root(SPM_Hierarchy *fs) {
    if (exists(fs->rootrec) != 0) {
        return 0;
    }
    return 1;
}

/**
 * Initialize (not populate) a spm root directory.
 * A "root record" is dropped into $ROOT/var
 * @param fs `SPM_Hierarchy` structure
 * @return success=0, error=-1
 */
int spm_hierarchy_make_root(SPM_Hierarchy *fs) {
    // Create the root directory if it does not exist
    if (mkdirs(fs->rootdir, 0755) != 0) {
        return -1;
    }

    if (touch(fs->rootrec) < 0) {
        return -1;
    }

    return 0;
}

/**
 * Generates a formatted string containing package information
 *
 * `%n` = Package name
 * `%v` = Package version
 * `%V` = Package version and revision (separated by a hyphen (`-`))
 * `%r` = Package revision
 * `%o` = Package origin
 * `%a` = Package filename on disk
 * `%c` = Package checksum (sha256)
 * `%s` = Package size (byte)
 * `%S` = Package size (human readable)
 *
 * @param package Pointer to `ManifestPackage`
 * @param fmt Format string
 * @return `malloc()`ed string
 */
char *spm_get_package_info_str(ManifestPackage *package, const char *fmt) {
    // Allocate at least enough space for the character arrays in ManifestPackage (there are several)
    char *output = calloc(1, sizeof(ManifestPackage));

    // Stores raw record data
    char tmp[SPM_PACKAGE_MEMBER_SIZE];

    // Stores width string (i.e. '%-12n' is parsed as '12')
    char str_width[10];

    // Record the maximum number of bytes to read
    size_t len_fmt = strlen(fmt);

    // Begin reading format string
    for (size_t i = 0; i < len_fmt; i++) {
        size_t width = 0;  // Default string padding amount
        int when = -1;  // When string padding is applied (-1 = none, 0 = before, 1 = after)
        char *hrs = NULL;  // Buffer for human_readable_size

        // Truncate temporary strings
        tmp[0] = '\0';
        str_width[0] = '\0';

        // Begin parsing formatter
        if (fmt[i] == '%') {
            // Advance to next character
            i++;

            // When the next character is a hyphen write string padding after the requested value
            if (fmt[i] == '-') {
                when = 1;
                i++;
            }

            // Is the next character a number?
            if (isdigit(fmt[i])) {
                if (when != 1) { // read as: "if padding not altered already"
                    when = 0;  // got no '-' but landed on some digits, so pad "before"
                }

                // Consume the numerical string and convert it to an integer
                int j = 0;
                while (isdigit(fmt[i])) {
                    memcpy(&str_width[j], &fmt[i], 1);
                    j++;
                    i++;
                }
                str_width[j] = 0;
                width = strtoul(str_width, NULL, 10);
            }

            // Retrieve information based on requested format character'
            switch (fmt[i]) {
                case 'n':
                    strcpy(tmp, package->name);
                    break;
                case 'v':
                    strcpy(tmp, package->version);
                    break;
                case 'V':
                    strcpy(tmp, package->version);
                    strcat(tmp, "-");
                    strcat(tmp, package->revision);
                    break;
                case 'r':
                    strcpy(tmp, package->revision);
                    break;
                case 'o':
                    strcpy(tmp, package->origin);
                    break;
                case 'a':
                    strcpy(tmp, package->archive);
                    break;
                case 'c':
                    strcpy(tmp, package->checksum_sha256);
                    break;
                case 's':
                    sprintf(tmp, "%zu", package->size);
                    break;
                case 'S':
                    hrs = human_readable_size(package->size);
                    strcpy(tmp, hrs);
                    free(hrs);
                    break;
                default:
                    // Formatter is not registered above. Oh well.
                    continue;
            }

            // Pad the string up to the length of the length of `tmp`
            int width_final = ((int)width - (int)strlen(tmp));

            // When `tmp` is longer than the requested width, use the original width
            if (width_final < 0) {
                width_final = width;
            }

            // Write padding "before" appending `tmp` to the output string
            if (when == 0) {
                for (size_t m = 0; m < width_final; m++) {
                    strcat(output, " ");
                }
            }

            // Append data to output string
            strcat(output, tmp);

            // Write padding "after" appending `tmp` to the output string
            if (when == 1) {
                for (size_t m = 0; m < width_final; m++) {
                    strcat(output, " ");
                }
            }
        } else {
            // Data was not parsed as a formatter, so append it to the output string as-is
            strncat(output, &fmt[i], 1);
        }
    }

    return output;
}

void spm_show_package(ManifestPackage *package) {
    char *output = NULL;

    if (package == NULL) {
        spmerrno = SPM_ERR_MANIFEST_INVALID;
        spmerrno_cause("ManifestPackage was NULL\n");
        return;
    }

    output = spm_get_package_info_str(package, "%-20n %-10V %8S %4o");

    if (output == NULL) {
        spmerrno = SPM_ERR_PKG_INVALID;
        spmerrno_cause("spm_get_package_info_str did not succeed");
        return;
    }
    puts(output);
    free(output);
}

void spm_show_package_manifest(Manifest *info) {
    char *output = NULL;

    if (info == NULL) {
        spmerrno = SPM_ERR_MANIFEST_INVALID;
        spmerrno_cause("Manifest was NULL\n");
        return;
    }

    for (size_t m = 0; m < info->records; m++) {
        spm_show_package(info->packages[m]);
    }

}
void spm_show_packages(ManifestList *info) {
    char *output = NULL;

    if (info == NULL) {
        spmerrno = SPM_ERR_MANIFEST_INVALID;
        spmerrno_cause("ManifestList was NULL");
        return;
    }

    for (size_t i = 0; i < manifestlist_count(info); i++) {
        spm_show_package_manifest(manifestlist_item(info, i));
    }
}

/**
 * Install a package and its dependencies into a destination root.
 * The destination is created if it does not exist.
 * @param _destroot directory to install package
 * @param _package name of archive to install (not a path)
 * @return success=0, exists=1, error=-1 (general), -2 (unable to create `destroot`)
 */
int spm_install(SPM_Hierarchy *fs, const char *tmpdir, const char *_package) {
    int status_tar;
    char *package = strdup(_package);

    if (!package) {
        fprintf(SYSERROR);
        return -1;
    }

    if (SPM_GLOBAL.verbose) {
        printf("Extracting archive: %s\n", package);
    }

    status_tar = 0;
    if ((status_tar = tar_extract_archive(package, tmpdir)) != 0) {
        fprintf(stderr, "Extraction program returned non-zero: %d: %s\n", status_tar, package);
        free(package);
        return -1;
    }

    free(package);
    return 0;
}

int spm_install_package_record(SPM_Hierarchy *fs, char *tmpdir, char *package_name) {
    RuntimeEnv *rt = runtime_copy(__environ);
    char *records_topdir = strdup(fs->dbrecdir);
    char *records_pkgdir = join((char *[]) {records_topdir, package_name, NULL}, DIRSEPS);
    char *descriptor = join((char *[]) {tmpdir, SPM_META_DESCRIPTOR, NULL}, DIRSEPS);
    char *filelist = join((char *[]) {tmpdir, SPM_META_FILELIST, NULL}, DIRSEPS);

    if (exists(records_pkgdir) != 0) {
        if (mkdirs(records_pkgdir, 0755) != 0) {
            return -1;
        }
    }

    if (exists(descriptor) != 0) {
        fprintf(stderr, "Missing: %s\n", descriptor);
        return 1;
    }

    if (exists(filelist) != 0) {
        fprintf(stderr, "Missing: %s\n", filelist);
        return 2;
    }

    if (rsync(NULL, descriptor, records_pkgdir) != 0) {
        fprintf(stderr, "Failed to copy '%s' to '%s'\n", descriptor, records_pkgdir);
        return 3;
    }

    if (rsync(NULL, filelist, records_pkgdir) != 0) {
        fprintf(stderr, "Failed to copy '%s' to '%s'\n", filelist, records_pkgdir);
        return 4;
    }

    free(records_topdir);
    free(records_pkgdir);
    free(descriptor);
    free(filelist);
    runtime_free(rt);
    return 0;
}

int spm_check_installed(SPM_Hierarchy *fs, char *package_name) {
    char *records_topdir = join((char *[]) {fs->localstatedir, "db", "records", NULL}, DIRSEPS);
    char *records_pkgdir = join((char *[]) {records_topdir, package_name, NULL}, DIRSEPS);

    char *descriptor = join((char *[]) {records_pkgdir, SPM_META_DESCRIPTOR, NULL}, DIRSEPS);
    char *filelist = join((char *[]) {records_pkgdir, SPM_META_FILELIST, NULL}, DIRSEPS);
    char **data = NULL;

    if ((exists(records_pkgdir) || exists(descriptor) || exists(descriptor)) != 0) {
        free(records_topdir);
        free(records_pkgdir);
        free(descriptor);
        free(filelist);
        return 0; // does not exist
    }

    data = spm_metadata_read(filelist, SPM_METADATA_VERIFY);
    if (data == NULL) {
        free(records_topdir);
        free(records_pkgdir);
        free(descriptor);
        free(filelist);
        return -1;
    }

    for (size_t i = 0; data[i] != NULL; i++) {
        free(data[i]);
    }
    free(data);

    free(records_topdir);
    free(records_pkgdir);
    free(descriptor);
    free(filelist);
    return 1; // exists
}

/**
 *
 * @return
 */
char *spm_install_fetch(const char *pkgdir, const char *_package) {
    char *package = strdup(_package);
    if (package == NULL) {
        perror("could not allocate memory for package name");
        fprintf(SYSERROR);
        return NULL;
    }

    long response = 0;
    char *url = strdup(package);
    char *payload = join_ex(DIRSEPS, pkgdir, basename(package), NULL);
    size_t tmp_package_len = strlen(payload);

    if (tmp_package_len > strlen(package)) {
        char *tmp = realloc(package, (tmp_package_len + 1) * sizeof(char));
        if (tmp == NULL) {
            perror("cannot realloc package path");
            return NULL;
        }
        package = tmp;
    }
    strcpy(package, payload);

    if (exists(payload) != 0) {
        if ((response = fetch(url, package)) >= 400) {
            fprintf(stderr, "HTTP(%ld): %s\n", response, http_response_str(response));
            return NULL;
        }
    }
    free(url);
    free(payload);

    return package;
}

/**
 * Perform a full package installation
 * @param mf
 * @param rootdir
 * @param packages
 * @return 0=success, -1=failed to create storage, -2=denied by user
 */
int spm_do_install(SPM_Hierarchy *fs, ManifestList *mf, StrList *packages) {
    size_t num_requirements = 0;
    ManifestPackage **requirements = NULL;
    char source[PATH_MAX];
    char *tmpdir = NULL;
    size_t package_count;

    package_count = strlist_count(packages);
    if (package_count == 0) {
        spmerrno = SPM_ERR_PKG_NOT_FOUND;
        spmerrno_cause("EMPTY PACKAGE LIST");
        return -1;
    }

    // Produce a dependency tree from requested package(s)
    for (size_t i = 0; i < package_count; i++) {
        char *item = strlist_item(packages, i);

        // Does the package exist in the manifest?
        if (manifestlist_search(mf, item) == NULL) {
            spmerrno = SPM_ERR_PKG_NOT_FOUND;
            spmerrno_cause(item);
            return -1;
        }

        requirements = resolve_dependencies(mf, item);
        if (requirements != NULL) {
            for (size_t c = num_requirements; requirements[c] != NULL; c++) {
                num_requirements++;
            }
        }
    }

    tmpdir = spm_mkdtemp(TMP_DIR, "spm_destroot", NULL);
    if (tmpdir == NULL) {
        perror("Could not create temporary destination root");
        fprintf(SYSERROR);
        return -1;
    }

    if (SPM_GLOBAL.verbose) {
        printf("Installation root: %s\n", fs->rootdir);
    }

    if (spm_hierarchy_make_root(fs) < 0) {
        spmerrno = SPM_ERR_ROOT_NO_RECORD;
        rmdirs(tmpdir);
        return -1;
    }

    // Install packages
    printf("Requested package(s):\n");
    for (size_t i = 0; requirements !=NULL && requirements[i] != NULL; i++) {
        spm_show_package(requirements[i]);
    }

    if (SPM_GLOBAL.prompt_user) {
        if (spm_prompt_user("Proceed with installation?", 1) == 0) {
            exit(-2);
        }
    }

    int fetched = 0;
    char *package_dir = strdup(SPM_GLOBAL.package_dir);
    for (size_t i = 0; requirements != NULL && requirements[i] != NULL; i++) {
        char *package_origin = calloc(PATH_MAX, sizeof(char));
        strncpy(package_origin, requirements[i]->origin, PATH_MAX);

        if (strstr(package_origin, SPM_GLOBAL.repo_target) == NULL) {
            if (!endswith(package_origin, DIRSEPS)) {
                strcat(package_origin, DIRSEPS);
            }
            strcat(package_origin, SPM_GLOBAL.repo_target);
        }

        char *package_path = join((char *[]) {package_origin, requirements[i]->archive, NULL}, DIRSEPS);
        char *package_localpath = join_ex(DIRSEPS, package_dir, requirements[i]->archive, NULL);
        free(package_origin);

        // Download the archive if necessary
        if (strstr(package_path, "://") != NULL && exists(package_localpath) != 0) {
            printf("Fetching: %s\n", package_path);
            package_path = spm_install_fetch(package_dir, package_path);
            if (package_path == NULL) {
                free(package_path);
                free(package_localpath);
                // TODO: set spmerrno here
                exit(1);
            }
            fetched = 1;
        }
        // Or copy the archive if necessary
        else {
            // TODO: Possibly an issue down the road, but not at the moment
            // You have another local manifest in use. Copy any used packages from there into the local package directory.
            if (exists(package_localpath) != 0 && strncmp(package_dir, package_path, strlen(package_dir)) != 0) {
                printf("Copying: %s\n", package_path);
                if (rsync(NULL, package_path, package_dir) != 0) {
                    fprintf(stderr, "Unable to copy: %s to %s\n", package_path, package_dir);
                    return -1;
                }
                fetched = 1;
            } else if (exists(package_localpath) != 0) {
                // All attempts to retrieve the requested package have failed. Die.
                fprintf(stderr, "Package manifest in '%s' claims '%s' exists, however it does not.\n", requirements[i]->origin, package_path);
                return -1;
            }
        }
        free(package_path);
        free(package_localpath);
    }

    // Update the package manifest
    if (fetched) {
        printf("Updating package manifest...\n");
        Manifest *tmp_manifest = manifest_from(SPM_GLOBAL.package_dir);
        manifest_write(tmp_manifest, package_dir);
        manifest_free(tmp_manifest);
    }

    if (spm_hierarchy_is_root(fs) == 0) {
        if (SPM_GLOBAL.verbose) {
            printf("Creating destination root: %s\n", fs->rootdir);
        }
    }

    printf("Installing package(s):\n");
    size_t num_installed = 0;
    for (size_t i = 0; requirements != NULL && requirements[i] != NULL; i++) {
        char *package_path = join((char *[]) {package_dir, requirements[i]->archive, NULL}, DIRSEPS);

        if (spm_check_installed(fs, requirements[i]->name)) {
            printf("  -> %s is already installed\n", requirements[i]->name);
            free(package_path);
            continue;
        }

        spm_show_package(requirements[i]);
        spm_install(fs, tmpdir, package_path);

        // Relocate installation root
        relocate_root(fs->rootdir, tmpdir);
        spm_install_package_record(fs, tmpdir, requirements[i]->name);
        num_installed++;
        free(package_path);
    }

    // free requirements array
    for (size_t i = 0; i < num_requirements; i++) {
        if (requirements[i] != NULL) {
            manifest_package_free(requirements[i]);
            requirements[i] = NULL;
        }
    }
    free(package_dir);

    if (num_installed != 0) {
        // Append a trailing slash to tmpdir to direct rsync to copy files, not the directory, into destroot
        sprintf(source, "%s%c", tmpdir, DIRSEP);

        // Remove metadata files before copying
        if (SPM_GLOBAL.verbose) {
            printf("Removing metadata\n");
        }
        spm_metadata_remove(source);

        // Copy temporary directory to destination
        if (SPM_GLOBAL.verbose) {
            printf("Installing tree: '%s' => '%s'\n", source, fs->rootdir);
        }

        if (rsync(NULL, source, fs->rootdir) != 0) {
            exit(1);
        }
    }

    if (SPM_GLOBAL.verbose) {
        printf("Removing temporary storage: '%s'\n", tmpdir);
    }
    rmdirs(tmpdir);
    return 0;
}
