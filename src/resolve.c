/**
 * Dependency resolution functions
 * @file resolve.c
 */
#include "spm.h"

static ManifestPackage *requirements[SPM_REQUIREMENT_MAX] = {0, };

/**
 * Scan global `requirements` array for `archive`
 * @param archive
 * @return 0 = not found, 1 = found
 */
int resolve_has_dependency(const char *archive) {
    for (size_t i = 0; requirements[i] != NULL && i < SPM_REQUIREMENT_MAX; i++) {
        if (strcmp(requirements[i]->archive, archive) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * Recursively scan a package for its dependencies
 * @param manifests `ManifestList` struct
 * @param spec Package name (accepts version specifiers)
 * @return success = array of `ManifestPackage`, not found = NULL
 */
ManifestPackage **resolve_dependencies(ManifestList *manifests, const char *spec) {
    ManifestPackage *package = manifestlist_search(manifests, spec);
    ManifestPackage *requirement = NULL;
    if (package == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < package->requirements_records && i < SPM_REQUIREMENT_MAX; i++) {
        requirement = manifestlist_search(manifests, package->requirements[i]);
        if (requirement == NULL) {
            break;
        }
        if (resolve_has_dependency(requirement->archive)) {
            free(requirement);
            continue;
        }
        resolve_dependencies(manifests, requirement->archive);
        requirements[i] = requirement;
    }
    free(package);
    return requirements;
}