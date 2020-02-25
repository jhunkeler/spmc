#ifndef SPM_INSTALL_H
#define SPM_INSTALL_H

int metadata_remove(const char *_path);
void install_show_package(ManifestPackage *package);
int install(const char *destroot, const char *tmpdir, const char *_package);
int install_package_record(char *from_root, char *package_name);
int is_installed(const char *rootdir, char *package_name);
int do_install(ManifestList *mf, const char *rootdir, StrList *packages);

#endif //SPM_INSTALL_H
