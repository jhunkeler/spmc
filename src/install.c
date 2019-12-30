/**
 * @file install.c
 */
#include "spm.h"

/**
 * SPM packages contain metadata files that are not useful post-install and would amount to a lot of clutter.
 * This function removes these data files from a directory tree
 * @param _path
 * @return success=0, error=-1
 */
int metadata_remove(const char *_path) {
    char *metadata[] = {
            SPM_META_DEPENDS,
            SPM_META_PREFIX_BIN,
            SPM_META_PREFIX_TEXT,
            SPM_META_MANIFEST,
            NULL,
    };

    if (exists(_path) != 0) {
        perror(_path);
        fprintf(SYSERROR);
        return -1;
    }

    for (int i = 0; metadata[i] != NULL; i++) {
        char path[PATH_MAX];
        sprintf(path, "%s%c%s", _path, DIRSEP, metadata[i]);
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
 * @param destroot directory to install package
 * @param _package name of archive to install (not a path)
 * @return success=0, error=-1 (general), -2 (unable to create `destroot`)
 */
int install(const char *destroot, const char *_package) {
    char *package = find_package(_package);
    if (!package) {
        fprintf(SYSERROR);
        return -1;
    }

    if (exists(destroot) != 0) {
        if (mkdirs(destroot, 0755) != 0) {
            fprintf(SYSERROR);
            return -2;
        }
    }

    char cwd[PATH_MAX];
    char source[PATH_MAX];
    char template[PATH_MAX];

    // circumvent -Wformat-truncation
    char *suffix = (char *) calloc(PATH_MAX, sizeof(char));
    if (!suffix) {
        perror("suffix");
        fprintf(SYSERROR);
    }
    strcpy(suffix, "spm_destroot_XXXXXX");
    snprintf(template, PATH_MAX, "%s%c%s", TMP_DIR, DIRSEP, suffix);
    free(suffix);

    // Create a new temporary directory and extract the requested package into it
    char *tmpdir = mkdtemp(template);
    tar_extract_archive(package, tmpdir);

    getcwd(cwd, sizeof(cwd));

    RelocationEntry **b_record = NULL;
    RelocationEntry **t_record = NULL;
    chdir(tmpdir);
    {
        // Rewrite binary prefixes
        b_record= prefixes_read(SPM_META_PREFIX_BIN);
        if (b_record) {
            for (int i = 0; b_record[i] != NULL; i++) {
                relocate(b_record[i]->path, b_record[i]->prefix, destroot);
            }
        }

        // Rewrite text prefixes
        t_record = prefixes_read(SPM_META_PREFIX_TEXT);
        if (t_record) {
            for (int i = 0; t_record[i] != NULL; i++) {
                file_replace_text(t_record[i]->path, t_record[i]->prefix, destroot);
            }
        }

        prefixes_free(b_record);
        prefixes_free(t_record);
    }
    chdir(cwd);

    // Append a trailing slash to tmpdir to direct rsync to copy files, not the directory, into destroot
    sprintf(source, "%s%c", tmpdir, DIRSEP);

    // Remove metadata files before copying
    metadata_remove(source);

    // Copy temporary directory to destination
    if (rsync(NULL, source, destroot) != 0) {
        exit(1);
    }
    rmdirs(tmpdir);

    free(package);
    return 0;
}
