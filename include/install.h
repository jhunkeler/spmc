#ifndef SPM_INSTALL_H
#define SPM_INSTALL_H

int metadata_remove(const char *_path);
void install_show_package(ManifestPackage *package);
int install(SPM_Hierarchy *fs, const char *tmpdir, const char *_package);
int install_package_record(SPM_Hierarchy *fs, char *tmpdir, char *package_name);
int is_installed(SPM_Hierarchy *fs, char *package_name);
int do_install(SPM_Hierarchy *fs, ManifestList *mf, StrList *packages);


#endif //SPM_INSTALL_H
