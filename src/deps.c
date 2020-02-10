/**
 * @file deps.c
 */
#include "spm.h"

/**
 *
 * @param deps
 * @param name
 * @return
 */
int dep_seen(Dependencies **deps, const char *name) {
    if (!deps) {
        return -1;
    }
    for (size_t i = 0; i != (*deps)->records; i++) {
        if (strstr((*deps)->list[i], name) != NULL) {
            return 1;
        }
    }
    return 0;
}

/**
 *
 * @param deps
 * @return
 */
int dep_init(Dependencies **deps) {
    (*deps) = (Dependencies *)calloc(1, sizeof(Dependencies));
    (*deps)->__size = 2;
    (*deps)->records = 0;
    (*deps)->list = (char **)calloc((*deps)->__size, sizeof(char *));
    if (!(*deps)->list) {
        return -1;
    }
    return 0;
}

/**
 *
 * @param deps
 */
void dep_free(Dependencies **deps) {
    for (size_t i = 0; i < (*deps)->records; i++) {
        free((*deps)->list[i]);
    }
    free((*deps));
}

/**
 *
 * @param deps
 * @param _name
 * @return
 */
int dep_append(Dependencies **deps, char *_name) {
    char *name = NULL;
    char *bname = NULL;

    if (!(*deps)) {
        return -1;
    }

    name = find_package(_name);
    if (!name) {
        perror(_name);
        fprintf(SYSERROR);
        return -1;
    }

    bname = basename(name);
    if (!bname) {
        perror(name);
        fprintf(SYSERROR);
        return -1;
    }

    (*deps)->__size++;
    (*deps)->list = (char **)realloc((*deps)->list, (sizeof(char *) * (*deps)->__size));
    if (!(*deps)->list) {
        free(name);
        return -1;
    }

    (*deps)->list[(*deps)->records] = strdup(bname);
    if (!(*deps)->list[(*deps)->records]) {
        free(name);
        return -1;
    }
    (*deps)->records++;

    free(name);
    return 0;
}

/**
 *
 * @param deps
 * @param filename
 * @return
 */
int dep_solve(Dependencies **deps, const char *filename) {
    if (!(*deps)) {
        return -1;
    }
    if (exists(filename) != 0) {
        return -1;
    }
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return -1;
    }

    char data[BUFSIZ];
    memset(data, '\0', sizeof(data));

    char *line = data;
    int line_count = 0;
    while (fgets(line, BUFSIZ, fp) != NULL) {
        size_t line_length = strlen(line);
        if (line_length > 1 && strstr(line, "\r\n")) {
            line[line_length - 2] = '\0';
        }
        if (line_length > 1 && line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0';
        }
        if (strcmp(line, "") == 0) {
            continue;
        }
        line_count++;
        if (dep_seen(deps, line) > 0) {
            // Already seen this dependency. Skip it.
            continue;
        }
        else {
            // Have not seen this dependency before
            if (dep_append(deps, line) == 0) {
                dep_solve(deps, line);
            }
        }
    }
    fclose(fp);
    return line_count;
}

/**
 *
 * @param deps
 * @param _package
 * @return
 */
int dep_all(Dependencies **deps, const char *_package) {
    static int next = 0;
    char *package = NULL;
    char depfile[PATH_MAX];
    char template[PATH_MAX];
    char *suffix = (char *)calloc(PATH_MAX, sizeof(char));

    memset(depfile, '\0', PATH_MAX);
    memset(template, '\0', PATH_MAX);
    strcpy(suffix, "spm_depends_all_XXXXXX");

    // Verify the requested package pattern exists
    package = find_package(_package);
    if (!package) {
        perror(_package);
        fprintf(SYSERROR);
        free(suffix);
        return -1;
    }

    // Create a new temporary directory and extract the requested package into it
    snprintf(template, PATH_MAX, "%s%c%s", TMP_DIR, DIRSEP, suffix);
    char *tmpdir = mkdtemp(template);
    if (!tmpdir) {
        perror(template);
        fprintf(SYSERROR);
        free(package);
        free(suffix);
        return -1;
    }
    if (tar_extract_file(package, ".SPM_DEPENDS", tmpdir) < 0) {
        perror(package);
        fprintf(SYSERROR);
        free(package);
        free(suffix);
        return -1;
    }

    // Scan depencency tree
    sprintf(depfile, "%s%c%s", tmpdir, DIRSEP, ".SPM_DEPENDS");
    int resolved = dep_solve(deps, depfile);

    // NOTE:
    //  1. `resolved` is the number of dependencies for the package we're scanning
    //  2. `next` permits us to converge on `resolved`, otherwise `i` would reset to `0` each time `dep_all` is called
    for (int i = next; i < resolved; i++) {
        next++;
        if (dep_seen(deps, (*deps)->list[i])) {
            dep_all(deps, (*deps)->list[i]);
        }
    }

    // Remove temporary data
    unlink(depfile);
    rmdir(tmpdir);
    free(package);
    free(suffix);
    return 0;
}

/**
 *
 * @param deps
 */
void dep_show(Dependencies **deps) {
    if ((*deps) == NULL) {
        return;
    }
    for (size_t i = 0; i < (*deps)->records; i++) {
        printf("  -> %s\n", (*deps)->list[i]);
    }
}
