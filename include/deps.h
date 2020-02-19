#ifndef SPM_DEPS_H
#define SPM_DEPS_H

typedef struct {
    size_t __size;      // Count of allocated records
    size_t records;     // Count of usable records
    char **list;        // Array of dependencies
} Dependencies;

// deps.c
int exists(const char *filename);
int dep_seen(Dependencies **deps, const char *name);
int dep_init(Dependencies **deps);
void dep_free(Dependencies **deps);
int dep_append(Dependencies **deps, const char *root, char *name);
int dep_solve(Dependencies **deps, const char *root, const char *filename);
int dep_all(Dependencies **deps, const char *root, const char *_package);
void dep_show(Dependencies **deps);

#endif //SPM_DEPS_H
