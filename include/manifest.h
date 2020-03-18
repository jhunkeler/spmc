/**
 * @file manifest.h
 */
#ifndef SPM_MANIFEST_H
#define SPM_MANIFEST_H

#define SHA256_DIGEST_STRING_LENGTH (SHA256_DIGEST_LENGTH * 2) + 1

#define SPM_MANIFEST_SEPARATOR '|'
#define SPM_MANIFEST_SEPARATOR_MAX 7
#define SPM_MANIFEST_NODATA "*"
#define SPM_MANIFEST_HEADER "# SPM PACKAGE MANIFEST"
#define SPM_MANIFEST_FILENAME "manifest.dat"

typedef struct {
    char **requirements;
    size_t requirements_records;
    size_t size;
    char archive[SPM_PACKAGE_MEMBER_SIZE];
    char name[SPM_PACKAGE_MEMBER_SIZE];
    char version[SPM_PACKAGE_MEMBER_SIZE];
    char revision[SPM_PACKAGE_MEMBER_SIZE];
    char checksum_sha256[SHA256_DIGEST_STRING_LENGTH];
    char origin[SPM_PACKAGE_MEMBER_ORIGIN_SIZE];
} ManifestPackage;

typedef struct {
    size_t records;
    ManifestPackage **packages;
    char origin[SPM_PACKAGE_MEMBER_ORIGIN_SIZE];
} Manifest;

typedef struct {
    size_t num_inuse;
    size_t num_alloc;
    Manifest **data;
} ManifestList;

int fetch(const char *url, const char *dest);
int manifest_package_cmp(ManifestPackage *a, ManifestPackage *b);
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

ManifestList *manifestlist_init();
Manifest *manifestlist_item(ManifestList *pManifestList, size_t index);
void manifestlist_set(ManifestList *pManifestList, size_t index, Manifest *manifest);
ManifestPackage *manifestlist_search(ManifestList *pManifestList, const char *_package);
size_t manifestlist_count(ManifestList *pManifestList);
void manifestlist_append(ManifestList *pManifestList, char* path);
void manifestlist_free(ManifestList *pManifestList);
#endif //SPM_MANIFEST_H
