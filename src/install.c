/**
 * @file install.c
 */
#include "spm.h"

extern const char *METADATA_FILES[];

/**
 * SPM packages contain metadata files that are not useful post-install and would amount to a lot of clutter.
 * This function removes these data files from a directory tree
 * @param _path
 * @return success=0, error=-1
 */
int metadata_remove(const char *_path) {
    if (exists(_path) != 0) {
        perror(_path);
        fprintf(SYSERROR);
        return -1;
    }

    for (int i = 0; METADATA_FILES[i] != NULL; i++) {
        char path[PATH_MAX];
        sprintf(path, "%s%c%s", _path, DIRSEP, METADATA_FILES[i]);
        if (exists(path) != 0) {
            continue;
        }
        if (unlink(path) < 0) {
            perror(path);
            fprintf(SYSERROR);
            return -1;
        }
    }
    return 0;
}

/**
 * Install a package and its dependencies into a destination root.
 * The destination is created if it does not exist.
 * @param _destroot directory to install package
 * @param _package name of archive to install (not a path)
 * @return success=0, error=-1 (general), -2 (unable to create `destroot`)
 */
int install(const char *destroot, const char *_package) {
    char *package = strdup(_package);
    if (!package) {
        fprintf(SYSERROR);
        return -1;
    }

    if (exists(destroot) != 0) {
        if (SPM_GLOBAL.verbose) {
            printf("Creating destination root: %s\n", destroot);
        }
        if (mkdirs(destroot, 0755) != 0) {
            fprintf(SYSERROR);
            return -2;
        }
    }

    char source[PATH_MAX];
    char template[PATH_MAX];

    // circumvent -Wformat-truncation
    char *suffix = (char *) calloc(PATH_MAX, sizeof(char));
    if (!suffix) {
        perror("suffix");
        fprintf(SYSERROR);
        return -1;
    }
    strcpy(suffix, "spm_destroot_XXXXXX");
    snprintf(template, PATH_MAX, "%s%c%s", TMP_DIR, DIRSEP, suffix);
    free(suffix);

    // Create a new temporary directory and extract the requested package into it
    char *tmpdir = mkdtemp(template);
    if (exists(tmpdir) != 0) {
        fprintf(stderr, "Failed to create temporary storage directory\n");
        fprintf(SYSERROR);
        exit(errno);
    }

    if (SPM_GLOBAL.verbose) {
        printf("Extracting archive: %s\n", package);
    }
    if (tar_extract_archive(package, tmpdir) != 0) {
        fprintf(stderr, "%s: %s\n", package, strerror(errno));
        return -1;
    }

    // Relocate temporary directory
    relocate_root(destroot, tmpdir);

    // Append a trailing slash to tmpdir to direct rsync to copy files, not the directory, into destroot
    sprintf(source, "%s%c", tmpdir, DIRSEP);

    // Remove metadata files before copying
    if (SPM_GLOBAL.verbose) {
        printf("Removing metadata\n");
    }
    metadata_remove(source);

    // Copy temporary directory to destination
    if (SPM_GLOBAL.verbose) {
        printf("Installing tree: '%s' => '%s'\n", source, destroot);
    }
    if (rsync(NULL, source, destroot) != 0) {
        exit(1);
    }

    if (SPM_GLOBAL.verbose) {
        printf("Removing temporary storage: '%s'\n", tmpdir);
    }
    rmdirs(tmpdir);

    free(package);
    return 0;
}
