/**
 * @file install.c
 */
#include <url.h>
#include "spm.h"

void spm_install_show_package(ManifestPackage *package) {
    if (package == NULL) {
        fprintf(stderr, "ERROR: package was NULL\n");
        return;
    }
    printf("  -> %-10s %-10s (origin: %s)\n", package->name, package->version, package->origin);
}

/**
 * Install a package and its dependencies into a destination root.
 * The destination is created if it does not exist.
 * @param _destroot directory to install package
 * @param _package name of archive to install (not a path)
 * @return success=0, exists=1, error=-1 (general), -2 (unable to create `destroot`)
 */
int spm_install(SPM_Hierarchy *fs, const char *tmpdir, const char *_package) {
    char *package = strdup(_package);

    if (!package) {
        fprintf(SYSERROR);
        return -1;
    }

    if (exists(fs->rootdir) != 0) {
        if (SPM_GLOBAL.verbose) {
            printf("Creating destination root: %s\n", fs->rootdir);
        }
        if (mkdirs(fs->rootdir, 0755) != 0) {
            fprintf(SYSERROR);
            free(package);
            return -2;
        }
    }

    if (SPM_GLOBAL.verbose) {
        printf("Extracting archive: %s\n", package);
    }

    if (tar_extract_archive(package, tmpdir) != 0) {
        fprintf(stderr, "%s: %s\n", package, strerror(errno));
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
    char *tmpdir = spm_mkdtemp("spm_destroot", NULL);

    if (tmpdir == NULL) {
        perror("Could not create temporary destination root");
        fprintf(SYSERROR);
        return -1;
    }

    if (SPM_GLOBAL.verbose) {
        printf("Installation root: %s\n", fs->rootdir);
    }

    // Produce a dependency tree from requested package(s)
    for (size_t i = 0; i < strlist_count(packages); i++) {
        char *item  = strlist_item(packages, i);
        requirements = resolve_dependencies(mf, item);
        if (requirements != NULL) {
            for (size_t c = num_requirements; requirements[c] != NULL; c++) {
                num_requirements++;
            }
        }
    }

    // Append requested package(s) to requirements array
    for (size_t i = 0; i < strlist_count(packages); i++) {
        char *name = strlist_item(packages, i);
        ManifestPackage *package = manifestlist_search(mf, name);

        if (package == NULL) {
            fprintf(stderr, "Package not found: %s\n", name);
            return -1;
        }

        requirements[i + num_requirements] = package;
    }

    // Install packages
    printf("Requested package(s):\n");
    for (size_t i = 0; requirements !=NULL && requirements[i] != NULL; i++) {
        spm_install_show_package(requirements[i]);
    }

    if (SPM_GLOBAL.prompt_user) {
        if (spm_prompt_user("Proceed with installation?", 1) == 0) {
            exit(-2);
        }
    }

    int fetched = 0;
    char *package_dir = strdup(SPM_GLOBAL.package_dir);
    for (size_t i = 0; requirements != NULL && requirements[i] != NULL; i++) {
        char *package_path = join((char *[]) {requirements[i]->origin, requirements[i]->archive, NULL}, DIRSEPS);
        char *package_localpath = join_ex(DIRSEPS, package_dir, requirements[i]->archive, NULL);

        // Download the archive if necessary
        if (strstr(package_path, "://") != NULL && exists(package_localpath) != 0) {
            printf("Fetching: %s\n", package_path);
            package_path = spm_install_fetch(package_dir, package_path);
            if (package_path == NULL) {
                free(package_path);
                free(package_localpath);
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

    printf("Installing package(s):\n");
    size_t num_installed = 0;
    for (size_t i = 0; requirements != NULL && requirements[i] != NULL; i++) {
        char *package_path = join((char *[]) {package_dir, requirements[i]->archive, NULL}, DIRSEPS);

        if (spm_check_installed(fs, requirements[i]->name)) {
            printf("  -> %s is already installed\n", requirements[i]->name);
            free(package_path);
            continue;
        }

        spm_install_show_package(requirements[i]);
        spm_install(fs, tmpdir, package_path);
        spm_install_package_record(fs, tmpdir, requirements[i]->name);
        num_installed++;
        free(package_path);
    }

    // free requirements array
    for (size_t i = 0; requirements != NULL && requirements[i] != NULL; i++) {
        manifest_package_free(requirements[i]);
    }
    free(package_dir);

    if (num_installed != 0) {
        // Relocate installation root
        relocate_root(fs->rootdir, tmpdir);

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
