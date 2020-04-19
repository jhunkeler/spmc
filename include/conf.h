/**
 * @file conf.h
 */
#ifndef SPM_CONF_H
#define SPM_CONF_H

#define CONFIG_BUFFER_SIZE 1024
#define PKG_DIR SPM_GLOBAL.package_dir
#define TMP_DIR SPM_GLOBAL.tmp_dir

typedef struct {
    char *key;
    char *value;
    size_t key_length;
    size_t value_length;
} ConfigItem;

typedef struct {
    char *rootrec;
    char *rootdir;
    char *bindir;
    char *includedir;
    char *libdir;
    char *datadir;
    char *mandir;
    char *sysconfdir;
    char *tmpdir;
    char *localstatedir;
    char *dbdir;        // $localstate/db
    char *dbrecdir;     // $localstate/db/records
} SPM_Hierarchy;

typedef struct {
    char *package_dir;
    char *tmp_dir;
    char *package_manifest;
    char *mirror_config;
    char **mirror_list;
    char *repo_target;
    char *user_config_basedir;
    char *user_config_file;
    int verbose;
    int prompt_user;
    ConfigItem **config;
    struct utsname sysinfo;
    SPM_Hierarchy fs;
} spm_vars;

ConfigItem **config_read(const char *filename);
ConfigItem *config_get(ConfigItem **item, const char *key);
void config_free(ConfigItem **item);
void config_test(void);

char *get_user_conf_dir(void);
char *get_user_config_file(void);
char *get_user_tmp_dir(void);
char *get_user_package_dir(void);
char *get_package_manifest(void);

void init_config_global(void);
void free_global_config(void);
void show_global_config(void);
void check_runtime_environment(void);

SPM_Hierarchy *spm_hierarchy_init(char *basepath);
void spm_hierarchy_free(SPM_Hierarchy *fs);

#endif //SPM_CONF_H
