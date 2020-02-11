#ifndef SPM_MANIFEST_H
#define SPM_MANIFEST_H

#define SHA256_DIGEST_STRING_LENGTH (SHA256_DIGEST_LENGTH * 2) + 1

#define PACKAGE_MEMBER_SIZE 0xff
#define PACKAGE_MEMBER_ORIGIN_SIZE PATH_MAX
#define PACKAGE_MEMBER_SEPARATOR '-'
#define PACKAGE_MEMBER_SEPARATOR_PLACEHOLD '*'

#define SPM_MANIFEST_SEPARATOR '|'
#define SPM_MANIFEST_SEPARATOR_MAX 7
#define SPM_MANIFEST_NODATA "*"
#define SPM_MANIFEST_HEADER "# SPM PACKAGE MANIFEST"
#define SPM_MANIFEST_FILENAME "manifest.dat"
#define SPM_PACKAGE_EXTENSION ".tar.gz"

typedef struct {
    char **requirements;
    size_t requirements_records;
    size_t size;
    char archive[PACKAGE_MEMBER_SIZE];
    char name[PACKAGE_MEMBER_SIZE];
    char version[PACKAGE_MEMBER_SIZE];
    char revision[PACKAGE_MEMBER_SIZE];
    char checksum_sha256[SHA256_DIGEST_STRING_LENGTH];
    char origin[PACKAGE_MEMBER_ORIGIN_SIZE];
} ManifestPackage;

typedef struct {
    size_t records;
    ManifestPackage **packages;
    char origin[PACKAGE_MEMBER_ORIGIN_SIZE];
} Manifest;

int fetch(const char *url, const char *dest);
void manifest_package_separator_swap(char **name);
void manifest_package_separator_restore(char **name);
Manifest *manifest_from(const char *package_dir);
Manifest *manifest_read(char *file_or_url);
int manifest_write(Manifest *info, const char *dest);
void manifest_free(Manifest *info);
void manifest_package_free(ManifestPackage *info);
ManifestPackage *manifest_search(Manifest *info, const char *package);
ManifestPackage *find_by_strspec(Manifest *manifest, const char *_strspec);
ManifestPackage *manifest_package_copy(ManifestPackage *manifest);

#endif //SPM_MANIFEST_H
