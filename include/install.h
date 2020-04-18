/**
 * @file install.h
 */
#ifndef SPM_INSTALL_H
#define SPM_INSTALL_H

int spm_hierarchy_is_root(SPM_Hierarchy *fs);
int spm_hierarchy_make_root(SPM_Hierarchy *fs);
void spm_install_show_package(ManifestPackage *package);
int spm_install(SPM_Hierarchy *fs, const char *tmpdir, const char *_package);
int spm_install_package_record(SPM_Hierarchy *fs, char *tmpdir, char *package_name);
int spm_check_installed(SPM_Hierarchy *fs, char *package_name);
int spm_do_install(SPM_Hierarchy *fs, ManifestList *mf, StrList *packages);

#endif //SPM_INSTALL_H
