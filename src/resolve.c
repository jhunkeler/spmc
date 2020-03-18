/**
 * Dependency resolution functions
 * @file resolve.c
 */
#include "spm.h"

static ManifestPackage *requirements[SPM_REQUIREMENT_MAX] = {0, };

void resolve_free() {
    for (size_t i = 0; i < SPM_REQUIREMENT_MAX; i++) {
        if (requirements[i] != NULL)
            manifest_package_free(requirements[i]);
    }
}

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
    static size_t req_i = 0;
    ManifestPackage *package = manifestlist_search(manifests, spec);
    ManifestPackage *requirement = NULL;

    if (package == NULL) {
        return requirements;
    }

    for (size_t i = 0; i < package->requirements_records && i < SPM_REQUIREMENT_MAX; i++) {
        requirement = manifestlist_search(manifests, package->requirements[i]);
        if (requirement == NULL) {
            fprintf(stderr, "ERROR: unable to resolve package via manifestlist_search(): '%s'\n", package->requirements[i]);
            exit(1);
        }
        if (resolve_has_dependency(requirement->archive)) {
            free(requirement);
        } else {
            resolve_dependencies(manifests, requirement->archive);
            requirements[req_i] = requirement;
        }
    }

    if (!resolve_has_dependency(package->archive)) {
        requirements[req_i] = package;
        req_i++;
    }

    return requirements;
}