#include "spm.h"

int install(const char *destroot, const char *_package) {
    char *package = find_package(_package);
    if (!package) {
        fprintf(SYSERROR);
        return -1;
    }
    printf("Installing: %s\n", package);
    if (access(destroot, F_OK) != 0) {
        if (mkdirs(destroot, 0755) != 0) {
            fprintf(SYSERROR);
            return -2;
        }
    }

    char cwd[PATH_MAX];
    char source[PATH_MAX];
    char template[PATH_MAX];
    char suffix[PATH_MAX] = "spm_destroot_XXXXXX";
    sprintf(template, "%s%c%s", TMP_DIR, DIRSEP, suffix);

    // Create a new temporary directory and extract the requested package into it
    char *tmpdir = mkdtemp(template);
    tar_extract_archive(package, tmpdir);

    getcwd(cwd, sizeof(cwd));

    RelocationEntry **b_record = NULL;
    RelocationEntry **t_record = NULL;
    chdir(tmpdir);
    {
        // Rewrite binary prefixes
        RelocationEntry **b_record = prefixes_read(".SPM_PREFIX_BIN");
        if (b_record) {
            for (int i = 0; b_record[i] != NULL; i++) {
                relocate(b_record[i]->path, b_record[i]->prefix, destroot);
            }
        }

        // Rewrite text prefixes
        RelocationEntry **t_record = prefixes_read(".SPM_PREFIX_TEXT");
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
    if (rsync(NULL, source, destroot) != 0) {
        exit(1);
    }
    rmdirs(tmpdir);

    free(package);
}
