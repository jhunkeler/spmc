/**
 * @file rpath.c
 */
#include "spm.h"

/**
 * Wrapper function to execute `patchelf` with arguments
 * @param _filename Path of file to modify
 * @param _args Arguments to pass to `patchelf`
 * @return success=Process struct, failure=NULL
 */
Process *patchelf(const char *_filename, const char *_args) {
    char *filename = strdup(_filename);
    char *args = strdup(_args);
    Process *proc_info = NULL;
    char sh_cmd[PATH_MAX];
    sh_cmd[0] = '\0';

    strchrdel(args, "&;|");
    strchrdel(filename, "&;|");
    sprintf(sh_cmd, "patchelf %s %s 2>&1", args, filename);

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
    int result = 1;     // default: not found

    char *filename = strdup(_filename);
    if (!filename) {
        return -1;
    }

    // sanitize input path
    strchrdel(filename, "&;|");

    Process *pe = patchelf(filename, "--print-rpath");
    strip(pe->output);
    if (!isempty(pe->output)) {
        result = 0;
    }
    else {
        // something went wrong with patchelf other than
        // what we're looking for
        result = -1;
    }

    free(filename);
    shell_free(pe);
    return result;
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
    char *filename = strdup(_filename);
    if (!filename) {
        return NULL;
    }
    char *path = strdup(filename);
    if (!path) {
        free(filename);
        return NULL;
    }

    char *rpath = NULL;

    // sanitize input path
    strchrdel(path, "&;|");

    Process *pe = patchelf(filename, "--print-rpath");
    if (pe->returncode != 0) {
        fprintf(stderr, "patchelf error: %s %s\n", path, strip(pe->output));
        return NULL;
    }

    rpath = (char *)calloc(strlen(pe->output) + 1, sizeof(char));
    if (!rpath) {
        free(filename);
        free(path);
        shell_free(pe);
        return NULL;
    }

    strncpy(rpath, pe->output, strlen(pe->output));
    strip(rpath);

    free(filename);
    free(path);
    shell_free(pe);
    return rpath;
}

/**
 * Generate a RPATH in the form of:
 *
 * `$ORIGIN/relative/path/to/lib/from/_filename/path`
 *
 * @param _filename
 * @return
 */
char *rpath_generate(const char *_filename) {
    const char *origin = "$ORIGIN/";
    char *filename = realpath(_filename, NULL);
    if (!filename) {
        return NULL;
    }
    char *nearest_lib = rpath_autodetect(filename);
    if (!nearest_lib) {
        return NULL;
    }
    char *result = (char *)calloc(strlen(origin) + strlen(nearest_lib) + 1, sizeof(char));
    if (!result) {
        return NULL;
    }
    sprintf(result, "%s%s", origin, nearest_lib);
    free(filename);
    free(nearest_lib);
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
    char *rpath_orig = rpath_get(filename);
    if (!rpath_orig) {
        return -1;
    }

    // Are the original and new RPATH identical?
    if (strcmp(rpath_orig, rpath) == 0) {
        free(rpath_orig);
        return 0;
    }

    char args[PATH_MAX];
    memset(args, '\0', PATH_MAX);
    sprintf(args, "--set-rpath \"%s\"", rpath);

    Process *pe = patchelf(filename, args);
    if (pe != NULL) {
        returncode = pe->returncode;
    }
    shell_free(pe);
    free(rpath_orig);
    return returncode;
}

/**
 * Automatically detect the nearest lib directory and set the RPATH of an executable
 * @param filename
 * @param _rpath
 * @return
 */
int rpath_autoset(const char *filename) {
    int returncode = 0;

    char *rpath_new = rpath_generate(filename);
    if (!rpath_new) {
        return -1;
    }

    returncode = rpath_set(filename, rpath_new);
    free(rpath_new);

    return returncode;
}

/**
 * Using `filename` as a starting point, step backward through the filesystem looking for a lib directory
 * @param filename path to file (or a directory)
 * @return success=relative path from `filename` to nearest lib directory, failure=NULL
 */
char *rpath_autodetect(const char *filename) {
    int has_real_libdir = 0;
    char *rootdir = dirname(filename);
    char *start = realpath(rootdir, NULL);
    char *cwd = realpath(".", NULL);
    char *result = NULL;

    // Change directory to the requested root
    chdir(start);

    char *visit = calloc(PATH_MAX, sizeof(char));  // Current directory
    char tmp[PATH_MAX];         // Current directory with lib directory appended
    char relative[PATH_MAX];    // Generated relative path to lib directory
    char sep[2];                // Holds the platform's directory separator

    // Initialize character arrays;
    tmp[0] = '\0';
    relative[0] = '\0';
    sprintf(sep, "%c", DIRSEP);

    while(1) {
        // Where are we in the file system?
        if((visit = getcwd(NULL, PATH_MAX)) == NULL) {
            exit(errno);
        }
        // Using the current visit path, check if it contains a lib directory
        snprintf(tmp, PATH_MAX, "%s%c%s", visit, DIRSEP, "lib");
        if (access(tmp, F_OK) == 0) {
            strcat(relative, "lib");
            has_real_libdir = 1;        // gate for memory allocation below
            break;
        }
            // Reaching the top of the file system indicates our search for a lib directory failed
        else if (strcmp(visit, "/") == 0) {
            break;
        }

        // Assemble relative path step for this location
        strcat(relative, "..");
        strcat(relative, sep);

        // Step one directory level back
        chdir("..");
        free(visit);
    }

    // If we found a viable lib directory, allocate memory for it
    if (has_real_libdir) {
        result = (char *)calloc(strlen(relative) + 1, sizeof(char));
        if (!result) {
            chdir(cwd);     // return to calling directory
            return NULL;
        }
        // Copy character array data to the result
        strncpy(result, relative, strlen(relative));
    }

    chdir(cwd);     // return to calling directory
    free(rootdir);
    free(cwd);
    free(start);
    return result;
}
