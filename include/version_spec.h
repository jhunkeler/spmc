/**
 * @file version_spec.h
 */
#ifndef SPM_VERSION_SPEC_H
#define SPM_VERSION_SPEC_H

#define VERSION_MAX 8
#define VERSION_BASE 10
#define VERSION_DELIM "."
#define VERSION_DELIM_LOCAL "+"
#define VERSION_LOCAL_MAX 255
#define VERSION_OPERATORS " ~!=<>"
#define VERSION_NOOP 1 << 0
#define VERSION_EQ 1 << 1
#define VERSION_NE 1 << 2
#define VERSION_GT 1 << 3
#define VERSION_LT 1 << 4
#define VERSION_COMPAT 1 << 5

struct Version {
    char *local;
    uint64_t asInt;
    uint32_t part[VERSION_MAX];
};

struct Version *version_init();
int version_read(struct Version **version, char *s);
void version_info(struct Version *version);

uint64_t version_from(const char *str);
int version_spec_from(const char *op);
ManifestPackage **find_by_spec(const Manifest *manifest, const char *name, const char *op, const char *version_str);
int pep440_match(const char *version);
struct PEP440 *pep440_version(const char *version);

#endif //SPM_VERSION_SPEC_H
