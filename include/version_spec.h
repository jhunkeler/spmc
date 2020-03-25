/**
 * @file version_spec.h
 */
#ifndef SPM_VERSION_SPEC_H
#define SPM_VERSION_SPEC_H

#define VERSION_OPERATORS " ~!=<>"
#define VERSION_NOOP 1 << 0
#define VERSION_EQ 1 << 1
#define VERSION_NE 1 << 2
#define VERSION_GT 1 << 3
#define VERSION_LT 1 << 4
#define VERSION_COMPAT 1 << 5

// version_spec.c
char *version_suffix_get_alpha(char *str);
char *version_suffix_get_modifier(char *str);
int64_t version_suffix_modifier_calc(char *str);
int version_suffix_alpha_calc(char *str);
int64_t version_from(const char *version_str);
int version_spec_from(const char *op);
ManifestPackage **find_by_spec(const Manifest *manifest, const char *name, const char *op, const char *version_str);

#endif //SPM_VERSION_SPEC_H
