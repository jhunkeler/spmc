/**
 * @file config_global.c
 */
#include "spm.h"

char *get_user_conf_dir(void) {
    char *result = NULL;
    wordexp_t wexp;
    wordexp("~/.spm", &wexp, 0);
    if (wexp.we_wordc != 0) {
        result = (char *)calloc(strlen(wexp.we_wordv[0]) + 1, sizeof(char));
        if (!result) {
            wordfree(&wexp);
            return NULL;
        }
        strncpy(result, wexp.we_wordv[0], strlen(wexp.we_wordv[0]));
        if (access(result, F_OK) != 0) {
            mkdirs(result, 0755);
        }
    }
    wordfree(&wexp);
    return result;
}

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
        return NULL;
    }
    free(ucd);
    // Allocate and return path to configuration file
    return strdup(template);
}

char *get_user_tmp_dir(void) {
    char template[PATH_MAX];
    char *ucd = get_user_conf_dir();
    sprintf(template, "%s%c%s", ucd, DIRSEP, "tmp");

    if (access(template, F_OK) != 0) {
        if (mkdirs(template, 0755) != 0) {
            return NULL;
        }
    }

    free(ucd);
    return strdup(template);
}

char *get_user_package_dir(void) {
    char template[PATH_MAX];
    char *ucd = get_user_conf_dir();
    sprintf(template, "%s%c%s", ucd, DIRSEP, "pkgs");

    if (access(template, F_OK) != 0) {
        if (mkdirs(template, 0755) != 0) {
            return NULL;
        }
    }

    free(ucd);
    return strdup(template);
}

char *get_package_manifest(void) {
    char template[PATH_MAX];
    char *ucd = get_user_conf_dir();

    sprintf(template, "%s%c%s", ucd, DIRSEP, "manifest.dat");
    char *wtf = template;

    if (access(template, F_OK) != 0) {
        fprintf(stderr, "Package manifest not found: %s\n", template);
        Manifest *manifest = manifest_from(PKG_DIR);
        manifest_write(manifest);
        manifest_free(manifest);
    }

    free(ucd);
    return strdup(template);
}
/**
 * Check whether SPM has access to external programs it needs
 */
void check_runtime_environment(void) {
    int bad_rt = 0;
    char *required[] = {
            "patchelf",
            "rsync",
            "tar",
            "bash",
            "reloc",
            NULL,
    };
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

void init_config_global(void) {
    SPM_GLOBAL.user_config_basedir = NULL;
    SPM_GLOBAL.user_config_file = NULL;
    SPM_GLOBAL.package_dir = NULL;
    SPM_GLOBAL.tmp_dir = NULL;
    SPM_GLOBAL.package_manifest = NULL;
    SPM_GLOBAL.config = NULL;
    SPM_GLOBAL.verbose = 0;

    if (uname(&SPM_GLOBAL.sysinfo) != 0) {
        fprintf(SYSERROR);
        exit(1);
    }

    SPM_GLOBAL.user_config_basedir = get_user_conf_dir();
    SPM_GLOBAL.user_config_file = get_user_config_file();
    if (SPM_GLOBAL.user_config_file) {
        SPM_GLOBAL.config = config_read(SPM_GLOBAL.user_config_file);
    }

    ConfigItem *item = NULL;

    // Initialize temp directory
    item = config_get(SPM_GLOBAL.config, "tmp_dir");
    if (item) {
        SPM_GLOBAL.tmp_dir = item->value;
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
        SPM_GLOBAL.package_dir = item->value;
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
        SPM_GLOBAL.package_manifest = item->value;
        if (access(SPM_GLOBAL.package_manifest, F_OK) != 0) {
            fprintf(stderr, "Package manifest not found: %s\n", SPM_GLOBAL.package_manifest);
            Manifest *manifest = manifest_from(PKG_DIR);
            manifest_write(manifest);
            manifest_free(manifest);
        }
    }
    else {
        SPM_GLOBAL.package_manifest = get_package_manifest();
    }
}

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
    if (SPM_GLOBAL.config) {
        config_free(SPM_GLOBAL.config);
    }
}

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