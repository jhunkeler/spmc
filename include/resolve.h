/**
 * @file resolve.h
 */
#ifndef SPM_RESOLVE_H
#define SPM_RESOLVE_H

#define SPM_REQUIREMENT_MAX 1024

int resolve_has_dependency(const char *archive);
ManifestPackage **resolve_dependencies(ManifestList *manifests, const char *spec);

#endif //SPM_RESOLVE_H
