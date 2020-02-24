#ifndef SPM_INSTALL_H
#define SPM_INSTALL_H

int metadata_remove(const char *_path);
void install_show_package(ManifestPackage *package);
int install(const char *destroot, const char *_package);

#endif //SPM_INSTALL_H
