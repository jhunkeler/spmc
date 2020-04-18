#include "spm.h"

/**
 * Remove a package
 * @param fs `SPM_Hierarchy`
 * @param _package_name
 * @return
 */
int spm_purge(SPM_Hierarchy *fs, const char *_package_name) {
    size_t files_count = 0;
    char **_files_orig = NULL;
    char *path = NULL;
    char *package_name = strdup(_package_name);
    char *package_topdir = join((char *[]) {fs->dbrecdir, package_name, NULL}, DIRSEPS);
    char *descriptor = join((char *[]) {package_topdir, SPM_META_DESCRIPTOR, NULL}, DIRSEPS);
    char *filelist = join((char *[]) {package_topdir, SPM_META_FILELIST, NULL}, DIRSEPS);
    char *rootrec = join((char *[]){path, "var", ".spm_root"}, DIRSEPS);

    if (spm_check_installed(fs, package_name) == 0) {
        // package is not installed in this root
        free(package_name);
        free(package_topdir);
        free(descriptor);
        free(filelist);
        return 1;
    }

    ConfigItem **desc = spm_descriptor_read(descriptor);
    char *name = config_get(desc, "name")->value;
    char *version = config_get(desc, "version")->value;
    char *revision = config_get(desc, "revision")->value;

    printf("Removing package: %s-%s-%s\n", name, version, revision);
    _files_orig = spm_metadata_read(filelist, SPM_METADATA_VERIFY);
    for (size_t i = 0; _files_orig[i] != NULL; i++) {
        files_count++;
    }

    for (size_t i = 0; _files_orig[i] != NULL; i++) {
        path = calloc(PATH_MAX, sizeof(char));
        path = join((char *[]) {fs->rootdir, _files_orig[i], NULL}, DIRSEPS);
        if (SPM_GLOBAL.verbose) {
            printf("  -> %s\n", path);
        }
        if (exists(path) != 0) {
            printf("%s does not exist\n", path);
        } else {
            remove(path);
        }
        free(path);
    }
    rmdirs(package_topdir);

    free(package_name);
    free(package_topdir);
    free(descriptor);
    free(filelist);
    config_free(desc);
    return 0;
}

/**
 * Remove packages
 * @param fs `SPM_Hierarchy`
 * @param packages `StrList`
 * @return
 */
int spm_do_purge(SPM_Hierarchy *fs, StrList *packages) {
    int status_remove = 0;

    if (spm_hierarchy_is_root(fs) < 0) {
        spmerrno = SPM_ERR_ROOT_NO_RECORD;
        return -1;
    }

    printf("Removing package(s):\n");
    for (size_t i = 0; i < strlist_count(packages); i++) {
        char *package = strlist_item(packages, i);
        if (spm_check_installed(fs, package) == 0) {
            printf("%s is not installed\n", package);
            return -1;
        }
        printf("-> %s\n", package);
    }

    if (SPM_GLOBAL.prompt_user) {
        if (spm_prompt_user("Proceed with removal?", 1) == 0) {
            return -2;
        }
    }

    for (size_t i = 0; i < strlist_count(packages); i++) {
        if ((status_remove = spm_purge(fs, strlist_item(packages, i))) != 0) {
            printf("Failed");
            break;
        }
    }
    return status_remove;
}