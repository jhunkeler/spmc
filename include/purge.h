/**
 * @file purge.h
 */
#ifndef SPM_REMOVE_H
#define SPM_REMOVE_H

int spm_purge(SPM_Hierarchy *fs, const char *_package_name);
int spm_do_purge(SPM_Hierarchy *fs, StrList *packages);

#endif //SPM_REMOVE_H
