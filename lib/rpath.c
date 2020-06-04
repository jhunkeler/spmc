/**
 * @file rpath.c
 */
#include "spm.h"

/**
 * Wrapper function to execute `patchelf` with arguments
 * @param _filename Path to file
 * @param _args Arguments to pass to `patchelf`
 * @return success=Process struct, failure=NULL
 */
Process *patchelf(const char *_filename, const char *_args) {
    char *filename = strdup(_filename);
    char *args = strdup(_args);
    Process *proc_info = NULL;
    char sh_cmd[PATH_MAX];
    sh_cmd[0] = '\0';

    strchrdel(args, SHELL_INVALID);
    strchrdel(filename, SHELL_INVALID);
    sprintf(sh_cmd, "patchelf %s %s 2>&1", args, filename);

    if (SPM_GLOBAL.verbose > 1) {
        printf("         EXEC : %s\n", sh_cmd);
    }

    shell(&proc_info, SHELL_OUTPUT, sh_cmd);

    free(filename);
    free(args);
    return proc_info;
}

/**
 * Wrapper function to execute `install_name_tool` with arguments
 * @param _filename Path to file
 * @param _args Arguments to pass to `install_name_tool`
 * @return success=Process struct, failure=NULL
 */
Process *install_name_tool(const char *_filename, const char *_args) {
    char *filename = strdup(_filename);
    char *args = strdup(_args);
    Process *proc_info = NULL;
    char sh_cmd[PATH_MAX];
    sh_cmd[0] = '\0';

    strchrdel(args, SHELL_INVALID);
    strchrdel(filename, SHELL_INVALID);
    sprintf(sh_cmd, "install_name_tool %s %s 2>&1", args, filename);

    if (SPM_GLOBAL.verbose > 1) {
        printf("         EXEC : %s\n", sh_cmd);
    }

    shell(&proc_info, SHELL_OUTPUT, sh_cmd);

    free(filename);
    free(args);
    return proc_info;
}

/**
 * Determine whether a RPATH or RUNPATH is present in file
 *
 * TODO: Replace with OS-native solution(s)
 *
 * @param _filename path to executable or library
 * @return -1=OS error, 0=has rpath, 1=not found
 */
int has_rpath(const char *_filename) {
    char *rpath = NULL;

    if (_filename == NULL) {
        spmerrno = EINVAL;
        return -1;
    }

    if ((rpath = shlib_rpath(_filename)) == NULL) {
        return 1;
    };

    return 0;
}

/**
 * Returns a RPATH or RUNPATH if one is defined in `_filename`
 *
 * TODO: Replace with OS-native solution(s)
 *
 * @param _filename path to executable or library
 * @return RPATH string, NULL=error (caller is responsible for freeing memory)
 */
char *rpath_get(const char *_filename) {
    if ((has_rpath(_filename)) != 0) {
        return strdup("");
    }
    return shlib_rpath(_filename);
}

/**
 * Generate a RPATH in the form of:
 *
 * `$ORIGIN/relative/path/to/lib/from/_filename/path`
 *
 * @param _filename
 * @return
 */
char *rpath_generate(const char *_filename, FSTree *tree, const char *destroot) {
    char *filename = realpath(_filename, NULL);
    if (!filename) {
        return NULL;
    }

    char *result = rpath_autodetect(filename, tree, destroot);
    if (!result) {
        free(filename);
        return NULL;
    }

    free(filename);
    return result;
}

/**
 * Set the RPATH of an executable
 * @param filename
 * @param rpath
 * @return
 */
int rpath_set(const char *filename, const char *rpath) {
    int returncode = 0;
    char args[PATH_MAX];
    Process *pe = NULL;

    memset(args, '\0', PATH_MAX);
#if OS_LINUX
    sprintf(args, "--set-rpath '%s'", rpath);
    pe = patchelf(filename, args);
#elif OS_DARWIN
    sprintf(args, "-add-rpath '%s'", rpath);
    pe = install_name_tool(filename, args);
#elif OS_WINDOWS
    // TODO: assuming windows has a mechanism for changing runtime paths, do it here.
#endif

    if (pe != NULL) {
        returncode = pe->returncode;
    }
    shell_free(pe);
    return returncode;
}

/**
 * Automatically detect the nearest lib directory and set the RPATH of an executable
 * @param filename
 * @param _rpath
 * @return
 */
int rpath_autoset(const char *filename, FSTree *tree, const char *destroot) {
    int returncode = 0;

    char *rpath_new = rpath_generate(filename, tree, destroot);
    if (!rpath_new) {
        return -1;
    }

    returncode = rpath_set(filename, rpath_new);
    free(rpath_new);

    return returncode;
}

/**
 * Find shared libraries in a directory tree
 *
 * @param root directory
 * @return `FSTree`
 */
FSTree *rpath_libraries_available(const char *root) {
    FSTree *tree = fstree(root, (char *[]) {SPM_SHLIB_EXTENSION, NULL}, SPM_FSTREE_FLT_CONTAINS);
    if (tree == NULL) {
        perror(root);
        fprintf(SYSERROR);
        return NULL;
    }
    return tree;
}

/**
 * Compute a RPATH based on the location `filename` relative to the shared libraries it requires
 *
 * @param filename path to file (or a directory)
 * @return success=relative path from `filename` to nearest lib directory, failure=NULL
 */
char *rpath_autodetect(const char *filename, FSTree *tree, const char *destroot) {
    const char *origin;
    char *rootdir = dirname(filename);
    char *start = realpath(rootdir, NULL);
    char *cwd = realpath(".", NULL);
    char *result = NULL;

    char *visit = NULL;                 // Current directory
    char _relative[PATH_MAX] = {0,};    // Generated relative path to lib directory
    char *relative = _relative;         // Pointer to relative path
    size_t depth_to_root = 0;

#if OS_DARWIN
    origin = "@rpath";
#elif OS_LINUX
    origin = "$ORIGIN";
#else
    origin = NULL;
#endif

    // BUG: Perl dumps its shared library in a strange place.
    // This function returns `$ORIGIN/../../../CORE` which is not what we want to see.
    // TODO: We WANT to see this: `$ORIGIN/../lib/perl5/xx.xx.xx/<arch>/CORE` not just the basename()

    // Change directory to the requested root
    chdir(start);

    // Count the relative path distance between the location of the binary, and the top of the root
    visit = strdup(start);
    for (depth_to_root = 0; strcmp(tree->root, visit) != 0; depth_to_root++) {
        // Copy the current visit pointer
        char *prev = visit;
        // Walk back another directory level
        visit = dirname(visit);
        // Free previous visit pointer
        if (prev) free(prev);
    }
    free(visit);

    // return to calling directory
    chdir(cwd);

    StrList *libs = strlist_init();
    if (libs == NULL) {
        fprintf(stderr, "failed to initialize library StrList\n");
        fprintf(SYSERROR);
        return NULL;
    }

    StrList *libs_wanted = shlib_deps(filename);
    if (libs_wanted == NULL) {
        fprintf(stderr, "failed to retrieve list of share libraries from: %s\n", filename);
        fprintf(SYSERROR);
        return NULL;
    }

    for (size_t i = 0; i < strlist_count(libs_wanted); i++) {
        char *shared_library = strlist_item(libs_wanted, i);
        char *match = NULL;
        match = dirname(fstree_search(tree, shared_library));
        replace_text(match, tree->root, destroot);
        if (match != NULL) {
            // Ignore duplicates
            if (strstr_array(libs->data, match) == NULL) {
                strlist_append(libs, match);
            }
        }
    }

    /*
    for (size_t i = 0; i < strlist_count(libs_wanted); i++) {
        // zero out relative path string
        memset(_relative, '\0', sizeof(_relative));
        // Get the shared library name we are going to look for in the tree
        char *shared_library = strlist_item(libs_wanted, i);

        // Is the shared library in the tree?
        char *match = NULL;
        if ((match = dirname(fstree_search(tree, shared_library))) != NULL) {
            // Begin generating the relative path string
            //strcat(relative, origin);
            //strcat(relative, DIRSEPS);

            // Append the number of relative levels to the relative path string
            if (depth_to_root) {
                for (size_t d = 0; d < depth_to_root; d++) {
                    strcat(relative, "..");
                    strcat(relative, DIRSEPS);
                }
            } else {
                strcat(relative, "..");
                strcat(relative, DIRSEPS);
            }
            // Append the match to the relative path string
            strcat(relative, basename(match));

            // Append relative path to array of libraries (if it isn't already in there)
            if (strstr_array(libs->data, relative) == NULL) {
                char *final = realpath(relative, NULL);
                strlist_append(libs, final);
                free(final);
            }
        }
    }
     */

#if OS_LINUX
    // Some programs do not require local libraries provided by SPM (i.e. libc)
    // Inject "likely" defaults here
    /*
    if (strlist_count(libs) == 0) {
        strlist_append(libs, "$ORIGIN/../lib");
        strlist_append(libs, "$ORIGIN/../lib64");
    }
     */
#endif

    // Populate result string
    result = join(libs->data, ":");

    // Clean up
    strlist_free(libs);
    strlist_free(libs_wanted);
    free(rootdir);
    free(cwd);
    free(start);
    return result;
}
