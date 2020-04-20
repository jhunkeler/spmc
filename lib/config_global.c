/**
 * @file config_global.c
 */
#include "spm.h"

// GLOBAL
spm_vars SPM_GLOBAL;


/**
 * Get path to user's local configuration directory
 * (The path will be created if it doesn't exist)
 * @return
 */
char *get_user_conf_dir(void) {
    char *result = NULL;

    result = expandpath("~/.spm");
    if (result == NULL) {
        perror("get_user_conf_dir");
        return NULL;
    }

    if (access(result, F_OK) != 0) {
        mkdirs(result, 0755);
    }
    return result;
}

/**
 * Get path to user's local configuration file
 * @return
 */
char *get_user_config_file(void) {
    const char *filename = "spm.conf";
    char template[PATH_MAX];
    char *ucd = get_user_conf_dir();
    if (!ucd) {
        return NULL;
    }
    // Initialize temporary path
    template[0] = '\0';

    sprintf(template, "%s%c%s", ucd, DIRSEP, filename);
    if (access(template, F_OK) != 0) {
        // No configuration exists, so fail
        free(ucd);
        return NULL;
    }
    free(ucd);
    // Allocate and return path to configuration file
    return strdup(template);
}

/**
 * Determine location of temporary storage location
 * @return
 */
char *get_user_tmp_dir(void) {
    char *template = NULL;
    char *ucd = get_user_conf_dir();
    template = join_ex(DIRSEPS, ucd, "tmp", NULL);

    if (access(template, F_OK) != 0) {
        if (mkdirs(template, 0755) != 0) {
            return NULL;
        }
    }

    free(ucd);
    return template;
}

/**
 * Determine location of package directory
 * @return
 */
char *get_user_package_dir(void) {
    char *template = NULL;
    char *ucd = get_user_conf_dir();

    template = join_ex(DIRSEPS, ucd, "pkgs", SPM_GLOBAL.repo_target, NULL);

    if (access(template, F_OK) != 0) {
        if (mkdirs(template, 0755) != 0) {
            return NULL;
        }
    }

    free(ucd);
    return template;
}

/**
 * Determine location of the package manifest
 * @return
 */
char *get_package_manifest(void) {
    Manifest *manifest = NULL;
    char *template = NULL;
    char *ucd = get_user_conf_dir();

    //free(ucd);
    //return strdup(template);

    template = join_ex(DIRSEPS, SPM_GLOBAL.package_dir, SPM_MANIFEST_FILENAME, NULL);
    if (access(template, F_OK) != 0) {
        fprintf(stderr, "Package manifest not found: %s\n", template);
        manifest = manifest_from(PKG_DIR);
        if (manifest == NULL) {
            perror("manifest generator");
            fprintf(SYSERROR);
            free(ucd);
            return NULL;
        }
        manifest_write(manifest, PKG_DIR);
        manifest_free(manifest);
    }

    free(ucd);
    return template;
}


/**
 * Check whether SPM has access to external programs it needs
 */
void check_runtime_environment(void) {
    int bad_rt = 0;
    char *required[] = {
            "file",
#if defined(__linux) || defined(__linux__)
            "patchelf",
#elif defined(__APPLE__) && defined(__MACH__)
            "install_name_tool",
#elif defined(__WIN32__)
            // TODO: Does windows provide some kind of equivalent?
#endif
            "objdump",
            "rsync",
            "tar",
            "bash",
            "reloc",
            NULL,
    };

    if (getenv("SHELL") == NULL) {
        fprintf(stderr, "Required environment variable 'SHELL' is not defined\n");
        bad_rt = 1;
    }

    for (int i = 0; required[i] != NULL; i++) {
        char *result = find_executable(required[i]);
        if (!result) {
            fprintf(stderr, "Required program '%s' is not installed\n", required[i]);
            bad_rt = 1;
        }
        free(result);
    }
    if (bad_rt) {
        exit(1);
    }
}

/**
 * Define the structure of a SPM root hierarchy
 * @param basepath
 * @return
 */
SPM_Hierarchy *spm_hierarchy_init(char *basepath) {
    SPM_Hierarchy *fs = calloc(1, sizeof(SPM_Hierarchy));
    fs->rootdir = strdup(basepath);
    fs->bindir = join((char *[]) {fs->rootdir, "bin", NULL}, DIRSEPS);
    fs->includedir = join((char *[]) {fs->rootdir, "include", NULL}, DIRSEPS);
    fs->libdir = join((char *[]) {fs->rootdir, "lib", NULL}, DIRSEPS);
    fs->datadir = join((char *[]) {fs->rootdir, "share", NULL}, DIRSEPS);
    fs->localstatedir = join((char *[]) {fs->rootdir, "var", NULL}, DIRSEPS);
    fs->sysconfdir = join((char *[]) {fs->rootdir, "etc", NULL}, DIRSEPS);
    fs->mandir = join((char *[]) {fs->datadir, "man", NULL}, DIRSEPS);
    fs->tmpdir = join((char *[]) {fs->rootdir, "tmp", NULL}, DIRSEPS);
    fs->dbdir = join((char *[]) {fs->localstatedir, "db", NULL}, DIRSEPS);
    fs->dbrecdir = join((char *[]) {fs->dbdir, "records", NULL}, DIRSEPS);
    fs->rootrec = join((char *[]) {".spm_root", NULL}, DIRSEPS);

    return fs;
}

/**
 * Free SPM_Hierarchy structure
 * @param fs
 */
void spm_hierarchy_free(SPM_Hierarchy *fs) {
    free(fs->rootdir);
    free(fs->bindir);
    free(fs->includedir);
    free(fs->libdir);
    free(fs->datadir);
    free(fs->localstatedir);
    free(fs->sysconfdir);
    free(fs->mandir);
    free(fs->tmpdir);
    free(fs->dbdir);
    free(fs->dbrecdir);
    free(fs->rootrec);
    free(fs);
}

/**
 * Populate global configuration structure
 */
void init_config_global(void) {
    SPM_GLOBAL.user_config_basedir = NULL;
    SPM_GLOBAL.user_config_file = NULL;
    SPM_GLOBAL.package_dir = NULL;
    SPM_GLOBAL.tmp_dir = NULL;
    SPM_GLOBAL.package_manifest = NULL;
    SPM_GLOBAL.config = NULL;
    SPM_GLOBAL.verbose = 0;
    SPM_GLOBAL.repo_target = NULL;
    SPM_GLOBAL.mirror_list = NULL;
    SPM_GLOBAL.prompt_user = 1;

    if (uname(&SPM_GLOBAL.sysinfo) != 0) {
        fprintf(SYSERROR);
        exit(errno);
    }

    // Initialize filesystem paths structure
    SPM_GLOBAL.fs.bindir = calloc(strlen(SPM_PROGRAM_BIN) + 1, sizeof(char));
    SPM_GLOBAL.fs.includedir = calloc(strlen(SPM_PROGRAM_INCLUDE) + 1, sizeof(char));
    SPM_GLOBAL.fs.libdir = calloc(strlen(SPM_PROGRAM_LIB) + 1, sizeof(char));
    SPM_GLOBAL.fs.datadir = calloc(strlen(SPM_PROGRAM_DATA) + 1, sizeof(char));

    if (!SPM_GLOBAL.fs.bindir || !SPM_GLOBAL.fs.includedir
        || !SPM_GLOBAL.fs.libdir) {
        perror("Unable to allocate memory for global filesystem paths");
        fprintf(SYSERROR);
        exit(errno);
    }

    strcpy(SPM_GLOBAL.fs.bindir, SPM_PROGRAM_BIN);
    strcpy(SPM_GLOBAL.fs.includedir, SPM_PROGRAM_INCLUDE);
    strcpy(SPM_GLOBAL.fs.libdir, SPM_PROGRAM_LIB);
    strcpy(SPM_GLOBAL.fs.datadir, SPM_PROGRAM_DATA);
    SPM_GLOBAL.fs.mandir = join((char *[]) {SPM_PROGRAM_DATA, "man", NULL}, DIRSEPS);

    SPM_GLOBAL.user_config_basedir = get_user_conf_dir();
    SPM_GLOBAL.user_config_file = get_user_config_file();
    if (SPM_GLOBAL.user_config_file) {
        SPM_GLOBAL.config = config_read(SPM_GLOBAL.user_config_file);
    }

    ConfigItem *item = NULL;

    // Initialize repository target (i.e. repository path suffix)
    SPM_GLOBAL.repo_target = join((char *[]) {SPM_GLOBAL.sysinfo.sysname, SPM_GLOBAL.sysinfo.machine, NULL}, DIRSEPS);
    item = config_get(SPM_GLOBAL.config, "repo_target");
    if (item) {
        free(SPM_GLOBAL.repo_target);
        SPM_GLOBAL.repo_target = normpath(item->value);
    }

    // Initialize mirror list filename
    SPM_GLOBAL.mirror_config = join((char *[]) {SPM_GLOBAL.user_config_basedir, SPM_MIRROR_FILENAME, NULL}, DIRSEPS);
    item = config_get(SPM_GLOBAL.config, "mirror_config");
    if (item) {
        free(SPM_GLOBAL.mirror_config);
        SPM_GLOBAL.mirror_config = normpath(item->value);
    }

    if (SPM_GLOBAL.mirror_config != NULL) {
        SPM_GLOBAL.mirror_list = mirror_list(SPM_GLOBAL.mirror_config);
    }

    // Initialize temp directory
    item = config_get(SPM_GLOBAL.config, "tmp_dir");
    if (item) {
        SPM_GLOBAL.tmp_dir = strdup(item->value);
        if (access(SPM_GLOBAL.tmp_dir, F_OK) != 0) {
            if (mkdirs(SPM_GLOBAL.tmp_dir, 0755) != 0) {
                fprintf(stderr, "Unable to create global temporary directory: %s\n", SPM_GLOBAL.tmp_dir);
                fprintf(SYSERROR);
                exit(1);
            }
        }
    }
    else {
        SPM_GLOBAL.tmp_dir = get_user_tmp_dir();
    }

    // Initialize package directory
    item = config_get(SPM_GLOBAL.config, "package_dir");
    if (item) {
        SPM_GLOBAL.package_dir = calloc(PATH_MAX, sizeof(char)); //strdup(item->value);
        strncpy(SPM_GLOBAL.package_dir, item->value, PATH_MAX - 1);
        strcat(SPM_GLOBAL.package_dir, DIRSEPS);
        strcat(SPM_GLOBAL.package_dir, SPM_GLOBAL.repo_target);

        if (access(SPM_GLOBAL.package_dir, F_OK) != 0) {
            if (mkdirs(SPM_GLOBAL.package_dir, 0755) != 0) {
                fprintf(stderr, "Unable to create global package directory: %s\n", SPM_GLOBAL.package_dir);
                fprintf(SYSERROR);
                exit(1);
            }
        }
    }
    else {
        SPM_GLOBAL.package_dir = get_user_package_dir();
    }

    // Initialize package manifest
    item = config_get(SPM_GLOBAL.config, "package_manifest");
    if (item) {
        SPM_GLOBAL.package_manifest = strdup(item->value);
        if (access(SPM_GLOBAL.package_manifest, F_OK) != 0) {
            fprintf(stderr, "Package manifest not found: %s\n", SPM_GLOBAL.package_manifest);
            Manifest *manifest = manifest_from(PKG_DIR);
            manifest_write(manifest, SPM_GLOBAL.package_manifest);
            manifest_free(manifest);
        }
    }
    else {
        SPM_GLOBAL.package_manifest = get_package_manifest();
    }
}

/**
 * Free memory allocated for global configuration
 */
void free_global_config(void) {
    if (SPM_GLOBAL.package_dir) {
        free(SPM_GLOBAL.package_dir);
    }
    if (SPM_GLOBAL.tmp_dir) {
        free(SPM_GLOBAL.tmp_dir);
    }
    if (SPM_GLOBAL.package_manifest) {
        free(SPM_GLOBAL.package_manifest);
    }
    if (SPM_GLOBAL.user_config_basedir) {
        free(SPM_GLOBAL.user_config_basedir);
    }
    if (SPM_GLOBAL.user_config_file) {
        free(SPM_GLOBAL.user_config_file);
    }
    if (SPM_GLOBAL.repo_target) {
        free(SPM_GLOBAL.repo_target);
    }
    if (SPM_GLOBAL.mirror_config) {
        free(SPM_GLOBAL.mirror_config);
    }
    if (SPM_GLOBAL.mirror_list) {
        mirror_list_free(SPM_GLOBAL.mirror_list);
    }

    free(SPM_GLOBAL.fs.bindir);
    free(SPM_GLOBAL.fs.includedir);
    free(SPM_GLOBAL.fs.libdir);
    free(SPM_GLOBAL.fs.datadir);
    free(SPM_GLOBAL.fs.mandir);
    if (SPM_GLOBAL.config) {
        config_free(SPM_GLOBAL.config);
    }
}

/**
 * Display global configuration data
 */
void show_global_config(void) {
    printf("#---------------------------\n");
    printf("#---- SPM CONFIGURATION ----\n");
    printf("#---------------------------\n");
    printf("# base dir: %s\n", SPM_GLOBAL.user_config_basedir ? SPM_GLOBAL.user_config_basedir : "none (check write permission on home directory)");
    printf("# config file: %s\n", SPM_GLOBAL.user_config_file ? SPM_GLOBAL.user_config_file : "none");
    if (SPM_GLOBAL.user_config_file) {
        printf("# config file contents:\n");
        for (int i = 0; SPM_GLOBAL.config[i] != NULL; i++) {
            printf("#    -> %s: %s\n", SPM_GLOBAL.config[i]->key, SPM_GLOBAL.config[i]->value);
        }
    }
    printf("# package storage: %s\n", SPM_GLOBAL.package_dir);
    printf("# temp storage: %s\n", SPM_GLOBAL.tmp_dir);
    printf("# package manifest: %s\n", SPM_GLOBAL.package_manifest);
    printf("\n");
}
