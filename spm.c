/**
 * SPM - Simple Package Manager
 * @file spm.c
 */

#include "spm.h"
static int VERBOSE_MODE = 0;

char *get_user_conf_dir() {
    char *result = NULL;
    wordexp_t wexp;
    wordexp("~/.spm", &wexp, 0);
    if (wexp.we_wordc != 0) {
        result = (char *)calloc(strlen(wexp.we_wordv[0]) + 1, sizeof(char));
        if (!result) {
            wordfree(&wexp);
            NULL;
        }
        strncpy(result, wexp.we_wordv[0], strlen(wexp.we_wordv[0]));
        if (access(result, F_OK) != 0) {
            mkdirs(result, 0755);
        }
    }
    wordfree(&wexp);
    return result;
}

char *get_user_config_file() {
    const char *filename = "spm.conf";
    char *result = NULL;
    char tmp[PATH_MAX];
    char *ucb = get_user_conf_dir();
    if (!ucb) {
        return NULL;
    }
    // Initialize temporary path
    tmp[0] = '\0';

    sprintf(tmp, "%s%c%s", ucb, DIRSEP, filename);
    if (access(tmp, F_OK) != 0) {
        // No configuration exists, so fail
        return NULL;
    }

    // Allocate and return path to configuration file
    result = (char *)calloc(strlen(tmp) + 1, sizeof(char));
    if (!result) {
        return NULL;
    }
    strncpy(result, tmp, strlen(tmp));

    return result;
}

/**
 * A wrapper for `popen()` that executes non-interactive programs and reports their exit value.
 * To redirect stdout and stderr you must do so from within the `fmt` string
 *
 * ~~~{.c}
 * int fd = 1;  // stdout
 * const char *log_file = "log.txt";
 * Process *proc_info;
 * int status;
 *
 * // Send stderr to stdout
 * shell(&proc_info, SHELL_OUTPUT, "foo 2>&1");
 * // Send stdout and stderr to /dev/null
 * shell(&proc_info, SHELL_OUTPUT,"bar &>/dev/null");
 * // Send stdout from baz to log.txt
 * shell(&proc_info, SHELL_OUTPUT, "baz %d>%s", fd, log_file);
 * // Do not record or redirect output from any streams
 * shell(&proc_info, SHELL_DEFAULT, "biff");
 * ~~~
 *
 * @param Process uninitialized `Process` struct will be populated with process data
 * @param options change behavior of the function
 * @param fmt shell command to execute (accepts `printf` style formatters)
 * @param ... variadic arguments (used by `fmt`)
 */
void shell(Process **proc_info, u_int64_t option, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t bytes_read = 0;
    size_t i = 0;
    size_t new_buf_size = 0;
    clockid_t clkid = CLOCK_REALTIME;
    FILE *proc = NULL;

    (*proc_info) = (Process *)calloc(1, sizeof(Process));
    if (!(*proc_info)) {
        fprintf(SYSERROR);
        exit(errno);
    }
    (*proc_info)->returncode = -1;

    // outbuf needs to be an integer type because fgetc returns EOF (> char)
    int *outbuf = (int *)calloc(1, sizeof(int));
    if (!outbuf) {
        fprintf(SYSERROR);
        exit(errno);
    }
    char *cmd = (char *)calloc(PATH_MAX, sizeof(char));
    if (!cmd) {
        fprintf(SYSERROR);
        exit(errno);
    }

    vsnprintf(cmd, PATH_MAX, fmt, args);

    if (option & SHELL_BENCHMARK) {
        if (clock_gettime(clkid, &(*proc_info)->start_time) == -1) {
            perror("clock_gettime");
            exit(errno);
        }
    }

    proc = popen(cmd, "r");
    if (!proc) {
        return;
    }

    if (option & SHELL_BENCHMARK) {
        if (clock_gettime(clkid, &(*proc_info)->stop_time) == -1) {
            perror("clock_gettime");
            exit(errno);
        }
        (*proc_info)->time_elapsed = ((*proc_info)->stop_time.tv_sec - (*proc_info)->start_time.tv_sec)
                                   + ((*proc_info)->stop_time.tv_nsec - (*proc_info)->start_time.tv_nsec) / 1E9;
    }

    if (option & SHELL_OUTPUT) {
        (*proc_info)->output = (char *)calloc(BUFSIZ, sizeof(char));

        while ((*outbuf = fgetc(proc)) != EOF) {

            if (i >= BUFSIZ) {
                new_buf_size = BUFSIZ + (i + bytes_read);
                (*proc_info)->output = (char *)realloc((*proc_info)->output, new_buf_size);
                i = 0;
            }
            if (*outbuf) {
                (*proc_info)->output[bytes_read] = (char)*outbuf;
            }
            bytes_read++;
            i++;
        }
    }
    (*proc_info)->returncode = pclose(proc);
    va_end(args);
    free(outbuf);
    free(cmd);
}

/**
 * Free process resources allocated by `shell()`
 * @param proc_info `Process` struct
 */
void shell_free(Process *proc_info) {
    if (proc_info->output) {
        free(proc_info->output);
    }
    free(proc_info);
}

/**
 * Extract a single file from a tar archive into a directory
 *
 * @param archive path to tar archive
 * @param filename known path inside the archive to extract
 * @param destination where to extract file to (must exist)
 * @return
 */
int tar_extract_file(const char *archive, const char* filename, const char *destination) {
    Process *proc = NULL;
    int status;
    char cmd[PATH_MAX];

    sprintf(cmd, "tar xf %s %s -C %s 2>&1", archive, filename, destination);
    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        fprintf(SYSERROR);
        return -1;
    }

    status = proc->returncode;
    shell_free(proc);

    return status;
}

int tar_extract_archive(const char *_archive, const char *_destination) {
    Process *proc = NULL;
    int status;
    char cmd[PATH_MAX];

    char *archive = strdup(_archive);
    if (!archive) {
        fprintf(SYSERROR);
        return -1;
    }
    char *destination = strdup(_destination);
    if (!destination) {
        fprintf(SYSERROR);
        return -1;
    }

    // sanitize archive
    strchrdel(archive, "&;|");
    // sanitize destination
    strchrdel(destination, "&;|");

    sprintf(cmd, "tar xf %s -C %s 2>&1", archive, destination);
    shell(&proc, SHELL_OUTPUT, cmd);
    if (!proc) {
        fprintf(SYSERROR);
        return -1;
    }

    status = proc->returncode;
    shell_free(proc);

    return status;
}

/**
 * glob callback function
 * @param epath path to file that generated the error condition
 * @param eerrno the error condition
 * @return the error condition
 */
int errglob(const char *epath, int eerrno) {
    fprintf(stderr, "glob matching error: %s (%d)", epath, eerrno);
    return eerrno;
}

/**
 * Determine how many times the character `ch` appears in `sptr` string
 * @param sptr string to scan
 * @param ch character to find
 * @return count of characters found
 */
int num_chars(const char *sptr, int ch) {
    int result = 0;
    for (int i = 0; sptr[i] != '\0'; i++) {
        if (sptr[i] == ch) {
            result++;
        }
    }
    return result;
}

/**
 * Scan for `pattern` string at the beginning of `sptr`
 *
 * @param sptr string to scan
 * @param pattern string to search for
 * @return 0 = success, -1 = failure
 */
int startswith(const char *sptr, const char *pattern) {
    for (size_t i = 0; i < strlen(pattern); i++) {
        if (sptr[i] != pattern[i]) {
            return -1;
        }
    }
    return 0;
}

/**
 * Scan for `pattern` string at the end of `sptr`
 *
 * @param sptr string to scan
 * @param pattern string to search for
 * @return 0 = success, -1 = failure
 */
int endswith(const char *sptr, const char *pattern) {
    size_t sptr_size = strlen(sptr);
    size_t pattern_size = strlen(pattern);
    for (size_t s = sptr_size - pattern_size, p = 0 ; s < sptr_size; s++, p++) {
        if (sptr[s] != pattern[p]) {
            return -1;
        }
    }
    return 0;
}

/**
 * Converts Win32 path to Unix path, and vice versa
 *  - On UNIX, Win32 paths will be converted UNIX
 *  - On Win32, UNIX paths will be converted to Win32
 *
 * This function is platform dependent.
 *
 * @param path a system path
 * @return string (caller is responsible for `free`ing memory)
 */
char *normpath(const char *path) {
    char *result = strdup(path);
    char *tmp = result;

    while (*tmp) {
        if (*tmp == NOT_DIRSEP) {
            *tmp = DIRSEP;
            tmp++;
            continue;
        }
        tmp++;
    }
    return result;
}

/**
 * Deletes any characters matching `chars` from `sptr` string
 *
 * @param sptr string to be modified in-place
 * @param chars a string containing characters (e.g. " \n" would delete whitespace and line feeds)
 */
void strchrdel(char *sptr, const char *chars) {
    while (*sptr != '\0') {
        for (int i = 0; chars[i] != '\0'; i++) {
            if (*sptr == chars[i]) {
                memmove(sptr, sptr + 1, strlen(sptr));
            }
        }
        sptr++;
    }
}

/**
 * Find the integer offset of the first occurrence of `ch` in `sptr`
 *
 * ~~~{.c}
 * char buffer[255];
 * char string[] = "abc=123";
 * long int separator_offset = strchroff(string, '=');
 * for (long int i = 0; i < separator_offset); i++) {
 *     buffer[i] = string[i];
 * }
 * ~~~
 *
 * @param sptr string to scan
 * @param ch character to find
 * @return offset to character in string, or 0 on failure
 */
long int strchroff(const char *sptr, int ch) {
    char *orig = strdup(sptr);
    char *tmp = orig;
    long int result = 0;
    while (*tmp != '\0') {
        if (*tmp == ch) {
            break;
        }
        tmp++;
    }
    result = tmp - orig;
    free(orig);

    return result;
}

/**
 * This function scans `sptr` from right to left removing any matches to `suffix`
 * from the string.
 *
 * @param sptr string to be modified
 * @param suffix string to be removed from `sptr`
 */
void substrdel(char *sptr, const char *suffix) {
    if (!sptr || !suffix) {
        return;
    }
    size_t sptr_len = strlen(sptr);
    size_t suffix_len = strlen(suffix);
    intptr_t target_offset = sptr_len - suffix_len;

    // Prevent access to memory below input string
    if (target_offset < 0) {
        return;
    }

    // Create a pointer to
    char *target = sptr + target_offset;
    if (!strcmp(target, suffix)) {
        // Purge the suffix
        memset(target, '\0', suffix_len);
        // Recursive call continues removing suffix until it is gone
        strip(sptr);
    }
}

/**
 * Scan a directory for a file by name, or by wildcard
 *
 * @param root directory path to scan
 * @param filename file to find (wildcards accepted)
 * @return success=path to file, failure=NULL
 */
char *find_file(const char *root, const char *filename) {
    glob_t results;
    int glob_flags = 0;
    int match = 0;
    char *rootpath = NULL;
    char *path = NULL;

    // GUARD
    if (!root || !filename || strstr(filename, "..") || strstr(filename, "./")) {
        return NULL;
    }

    if (!(path = (char *)calloc(PATH_MAX + 1, sizeof(char)))) {
        fprintf(SYSERROR);
        exit(errno);
    }

    if (!(rootpath = realpath(root, NULL))) {
        return NULL;
    }

    strcat(path, rootpath);
    strcat(path, "/");
    strcat(path, filename);

    // Save a little time if the file exists
    if (access(path, F_OK) != -1) {
        return path;
    }

    // Inject wildcard
    strcat(path, "*");
    // Search for the file
    match = glob(path, glob_flags, errglob, &results);

    if (match != 0) {
        // report critical errors except GLOB_NOMATCH
        if (match == GLOB_NOSPACE || match == GLOB_ABORTED) {
            fprintf(SYSERROR);
        }
        return NULL;
    }

    // Resize path to the length of the first match
    char *want = results.gl_pathv[0];
    if (!(path = (char *)realloc(path, strlen(want) + 1))) {
        fprintf(SYSERROR);
        exit(errno);
    }

    // Replace path string with wanted path string
    strncpy(path, want, strlen(want));

    free(rootpath);
    globfree(&results);
    return path;
}

/**
 * Scan the package directory for a package by name
 * @param filename file to find
 * @return success=path to file, failure=NULL
 */
char *find_package(const char *filename) {
    return find_file(PKG_DIR, filename);
}

/**
 * Split a string by every delimiter in `delim` string.
 *
 * Callee must free memory using `split_free()`
 *
 * @param sptr string to split
 * @param delim characters to split on
 * @return success=parts of string, failure=NULL
 */
char** split(char *_sptr, const char* delim)
{
    size_t split_alloc = 0;
    // Duplicate the input string and save a copy of the pointer to be freed later
    char *orig = strdup(_sptr);
    char *sptr = orig;
    if (!sptr) {
        return NULL;
    }

    // Determine how many delimiters are present
    for (size_t i = 0; i < strlen(delim); i++) {
        split_alloc += num_chars(sptr, delim[i]);
    }
    // Preallocate enough records based on the number of delimiters
    char **result = (char **)calloc(split_alloc + 2, sizeof(char *));
    if (!result) {
        free(sptr);
        return NULL;
    }

    // Separate the string into individual parts and store them in the result array
    int i = 0;
    char *token = NULL;
    while((token = strsep(&sptr, delim)) != NULL) {
        result[i] = (char *)calloc(1, sizeof(char) * strlen(token) + 1);
        if (!result[i]) {
            free(sptr);
            return NULL;
        }
        strncpy(result[i], token, strlen(token));   // copy the string contents into the record
        i++;    // next record
    }
    free(orig);
    return result;
}

/**
 * Frees memory allocated by `split()`
 * @param ptr pointer to array
 */
void split_free(char **ptr) {
    for (int i = 0; ptr[i] != NULL; i++) {
        free(ptr[i]);
    }
    free(ptr);
}

/**
 * Extract the string encapsulated by characters listed in `delims`
 *
 * ~~~{.c}
 * char *str = "this is [some data] in a string";
 * char *data = substring_between(string, "[]");
 * // data = "some data";
 * ~~~
 *
 * @param sptr string to parse
 * @param delims two characters surrounding a string
 * @return success=text between delimiters, failure=NULL
 */
char *substring_between(char *sptr, const char *delims) {
    // Ensure we have enough delimiters to continue
    size_t delim_count = strlen(delims);
    if (delim_count != 2) {
        return NULL;
    }

    // Create pointers to the delimiters
    char *start = strpbrk(sptr, &delims[0]);
    char *end = strpbrk(sptr, &delims[1]);

    // Ensure the string has both delimiters
    if (!start || !end) {
        return NULL;
    }

    start++;    // ignore leading delimiter

    // Get length of the substring
    size_t length = end - start;

    char *result = (char *)calloc(length + 1, sizeof(char));
    if (!result) {
        return NULL;
    }

    // Copy the contents of the substring to the result
    char *tmp = result;
    while (start != end) {
        *tmp = *start;
        tmp++;
        start++;
    }

    return result;
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

    Process *proc_info = NULL;
    char *rpath = NULL;

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
char *get_rpath(const char *_filename) {
    if ((has_rpath(_filename)) != 0) {
        return NULL;
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

    Process *proc_info = NULL;
    char *rpath = NULL;

    // sanitize input path
    strchrdel(path, "&;|");

    Process *pe = patchelf(filename, "--print-rpath");
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
char *gen_rpath(const char *_filename) {
    const char *origin = "$ORIGIN/";
    char *filename = realpath(_filename, NULL);
    if (!filename) {
        return NULL;
    }
    char *nearest_lib = libdir_nearest(filename);
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


int set_rpath(const char *filename, char *_rpath) {
    int returncode;

    char *rpath_new = gen_rpath(filename);
    if (!rpath_new) {
        return -1;
    }

    char *rpath_orig = get_rpath(filename);
    if (!rpath_orig) {
        return -1;
    }

    // Are the original and new RPATH identical?
    if (strcmp(rpath_orig, rpath_new) == 0) {
        free(rpath_new);
        free(rpath_orig);
        return 0;
    }

    Process *pe = patchelf(filename, "--set-rpath");
    if (pe) {
        returncode = pe->returncode;
    }
    shell_free(pe);
    free(rpath_new);
    free(rpath_orig);
    return pe->returncode;
}

/**
 * Lists a directory (use `fstree` instead if you only want a basic recursive listing))
 * @param dirpath
 * @param result
 */
void walkdir(char *dirpath, Dirwalk **result, unsigned int dirs) {
    static int i = 0;
    static int locked = 0;
    size_t dirpath_size = strlen(dirpath);
    const size_t initial_records = 2;

    DIR *dp = opendir(dirpath);
    if (!dp) {
        return;
    }

    if (!locked) {
        (*result) = (Dirwalk *)reallocarray((*result),1, sizeof(Dirwalk));
        (*result)->paths = (char **)calloc(initial_records, sizeof(char *));
        i = 0;
        locked = 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        (*result)->paths = (char **) reallocarray((*result)->paths, (initial_records + i), sizeof(char *));
        char *name = entry->d_name;

        char path[PATH_MAX];
        char sep = DIRSEP;
        int path_size = snprintf(path, PATH_MAX, "%s%c%s", dirpath, sep, entry->d_name);

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }

        (*result)->paths[i] = (char *) calloc((size_t) (path_size + 1), sizeof(char));
        if (entry->d_type == DT_DIR) {
            if (dirs) {
                strncpy((*result)->paths[i], path, (size_t) path_size);
                i++;
            }
            dirpath[dirpath_size] = DIRSEP;
            strcpy(dirpath + dirpath_size + 1, name);
            walkdir(dirpath, result, dirs);
            dirpath[dirpath_size] = '\0';
        }
        else {
            strncpy((*result)->paths[i], path, (size_t) path_size);
            i++;
        }
    }
    (*result)->count = i;
    (*result)->paths[i] = NULL;
    closedir(dp);
    if (!strcmp(dirpath, "..") || !strcmp(dirpath, ".")) {
        locked = 0;
    }
}

/**
 * Generate a listing of all files under `path`
 * @param path
 * @return success=array of paths, failure=NULL
 */
char **fstree(const char *path, unsigned int get_dir_flag) {
    Dirwalk *dlist = NULL;
    char wpath[PATH_MAX];
    strcpy(wpath, path);

    if (access(wpath, F_OK) != 0) {
        return NULL;
    }

    walkdir(wpath, &dlist, get_dir_flag);
    char **result = (char **)calloc((size_t) (dlist->count + 1), sizeof(char *));
    for (int i = 0; dlist->paths[i] != NULL; i++) {
        result[i] = (char *)calloc(strlen(dlist->paths[i]) + 1, sizeof(char));
        if (!result[i]) {
            return NULL;
        }
        memcpy(result[i], dlist->paths[i], strlen(dlist->paths[i]));
        free(dlist->paths[i]);
    }
    strsort(result);

    free(dlist->paths);
    free(dlist);
    return result;
}

/*
 * Helper function for `strsort`
 */
static int _strsort_compare(const void *a, const void *b) {
    const char *aa = *(const char**)a;
    const char *bb = *(const char**)b;
    int result = strcmp(aa, bb);
    return result;
}

/**
 * Sort an array of strings alphabetically
 * @param arr
 */
void strsort(char **arr) {
    size_t arr_size = 0;

    // Determine size of array
    for (size_t i = 0; arr[i] != NULL; i++) {
        arr_size = i;
    }
    qsort(arr, arr_size, sizeof(char *), _strsort_compare);
}

/*
 * Helper function for `strsortlen`
 */
static int _strsortlen_asc_compare(const void *a, const void *b) {
    const char *aa = *(const char**)a;
    const char *bb = *(const char**)b;
    size_t len_a = strlen(aa);
    size_t len_b = strlen(bb);
    return len_a > len_b;
}

/*
 * Helper function for `strsortlen`
 */
static int _strsortlen_dsc_compare(const void *a, const void *b) {
    const char *aa = *(const char**)a;
    const char *bb = *(const char**)b;
    size_t len_a = strlen(aa);
    size_t len_b = strlen(bb);
    return len_a < len_b;
}
/**
 * Sort an array of strings by length
 * @param arr
 */
void strsortlen(char **arr, unsigned int sort_mode) {
    typedef int (*compar)(const void *, const void *);

    compar fn = _strsortlen_asc_compare;
    if (sort_mode != 0) {
        fn = _strsortlen_dsc_compare;
    }

    size_t arr_size = 0;

    // Determine size of array
    for (size_t i = 0; arr[i] != NULL; i++) {
        arr_size = i;
    }
    qsort(arr, arr_size, sizeof(char *), fn);
}

long int get_file_size(const char *filename) {
    long int result = 0;
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    result = ftell(fp);
    fclose(fp);
    return result;
}

int fstrstr(const char *filename, const char *pattern) {
    int result = 1;  // default "not found"

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return -1;
    }

    long int file_len = get_file_size(filename);
    if (file_len < 0) {
        return -1;
    }
    char *buffer = (char *)calloc((size_t) file_len, sizeof(char));
    if (!buffer) {
        return -1;
    }
    size_t pattern_len = strlen(pattern);

    fread(buffer, (size_t) file_len, sizeof(char), fp);
    fclose(fp);

    for (size_t i = 0; i < file_len; i++) {
        if (!memcmp(&buffer[i], pattern, pattern_len)) {
            result = 0;  // found
            break;
        }
    }
    free(buffer);
    return result;
}

/**
 * Attempt to create a directory (or directories)
 * @param _path A path to create
 * @param mode UNIX permissions (octal)
 * @return success=0, failure=-1 (+ errno will be set)
 */
int mkdirs(const char *_path, mode_t mode) {
    int result = 0;
    char *path = normpath(_path);
    char tmp[PATH_MAX];
    tmp[0] = '\0';

    char sep[2];
    sprintf(sep, "%c", DIRSEP);
    char **parts = split(path, sep);
    for (int i = 0; parts[i] != NULL; i++) {
        strcat(tmp, parts[i]);
        strcat(tmp, sep);
        if (access(tmp, F_OK) != 0) {
            result = mkdir(tmp, mode);
        }
    }
    split_free(parts);
    return result;
}

/**
 * Get the full path of a shell command
 * @param program
 * @return success=absolute path to program, failure=NULL
 */
char *find_executable(const char *program) {
    int found = 0;
    char *result = NULL;
    char *env_path = NULL;
    env_path = getenv("PATH");
    if (!env_path) {
        return NULL;
    }
    char **search_paths = split(env_path, ":");

    char buf[PATH_MAX];
    for (int i = 0; search_paths[i] != NULL; i++) {
        sprintf(buf, "%s%c%s", search_paths[i], DIRSEP, program);
        if (access(buf, F_OK | X_OK) == 0) {
            found = 1;
            break;
        }
        memset(buf, '\0', sizeof(buf));
    }
    if (found) {
        result = strdup(buf);
    }
    split_free(search_paths);
    return result;
}

/**
 * Check whether this program will run properly given the current runtime environment
 */
void check_runtime_environment(void) {
    int bad_rt = 0;
    char *required[] = {
        "patchelf",
        "rsync",
        "tar",
        "bash",
        NULL,
    };
    for (int i = 0; required[i] != NULL; i++) {
        char *result = find_executable(required[i]);
        if (!result) {
            fprintf(stderr, "Required program '%s' is not installed or on your PATH\n", required[i]);
            bad_rt = 1;
        }
        free(result);
    }
    if (bad_rt) {
        exit(1);
    }
}

/**
 * Strip file name from directory
 * Note: Caller is responsible for freeing memory
 *
 * @param _path
 * @return success=path to directory, failure=NULL
 */
char *dirname(const char *_path) {
    char *path = strdup(_path);
    char *last = strrchr(path, DIRSEP);
    if (!last) {
        return NULL;
    }
    // Step backward, stopping on the first non-separator
    // This ensures strings like "/usr//////" are converted to "/usr", but...
    // it will do nothing to fix up a path like "/usr//////bin/bash
    char *lookback = last;
    while (*(lookback - 1) == DIRSEP) {
        lookback--;
    }

    *lookback = '\0';
    return path;
}

/**
 * Strip directory from file name
 * Note: Caller is responsible for freeing memory
 *
 * @param _path
 * @return success=file name, failure=NULL
 */
char *basename(const char *_path) {
    char *result = NULL;
    char *path = strdup(_path);
    char *last = strrchr(path, DIRSEP);
    if (!last) {
        return NULL;
    }

    // Perform a lookahead ensuring the string is valid beyond the last separator
    if ((last + 1) != NULL) {
        result = strdup(last + 1);
    }
    free(path);

    return result;
}

/**
 * Using `filename` as a starting point, step backward through the filesystem looking for a lib directory
 * @param filename path to file (or a directory)
 * @return success=relative path from `filename` to nearest lib directory, failure=NULL
 */
char *libdir_nearest(const char *filename) {
    int has_real_libdir = 0;
    char *rootdir = dirname(filename);
    char *start = realpath(rootdir, NULL);
    char *cwd = realpath(".", NULL);
    char *result = NULL;

    // Change directory to the requested root
    chdir(start);

    char visit[PATH_MAX];       // Current directory
    char tmp[PATH_MAX];         // Current directory with lib directory appended
    char relative[PATH_MAX];    // Generated relative path to lib directory
    char sep[2];                // Holds the platform's directory separator

    // Initialize character arrays;
    visit[0] = '\0';
    tmp[0] = '\0';
    relative[0] = '\0';
    sprintf(sep, "%c", DIRSEP);

    while(1) {
        // Where are we in the file system?
        getcwd(visit, sizeof(visit));
        // Using the current visit path, check if it contains a lib directory
        sprintf(tmp, "%s%clib", visit, DIRSEP);
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
    sprintf(sh_cmd, "patchelf %s %s", args, filename);

    shell(&proc_info, SHELL_OUTPUT, sh_cmd);

    free(filename);
    free(args);
    return proc_info;
}


int rmdirs(const char *_path) {
    char *path = strdup(_path);
    if (!path) {
        return -1;
    }
    if (access(path, F_OK) != 0) {
        fprintf(SYSERROR);
        free(path);
        return -1;
    }

    char **files = fstree(path, 1);
    if (!files) {
        free(path);
        return -1;
    }

    while (access(path, F_OK) == 0) {
        for (int i = 0; files[i] != NULL; i++) {
            remove(files[i]);
        }
        remove(path);
    }

    for (int i = 0; files[i] != NULL; i++) {
        free(files[i]);
    }

    free(files);
    free(path);
    return 0;
}

int install(const char *destroot, const char *_package) {
    char *package = find_package(_package);
    if (!package) {
        fprintf(SYSERROR);
        return -1;
    }
    if (access(destroot, F_OK) != 0) {
        if (mkdirs(destroot, 0755) != 0) {
            fprintf(SYSERROR);
            return -2;
        }
    }

    char template[PATH_MAX];
    char suffix[PATH_MAX] = "spm_destroot_XXXXXX";
    char *ucd = get_user_conf_dir();
    sprintf(template, "%s%c%s", ucd, DIRSEP, suffix);
    char *tmpdir = mkdtemp(template);
    tar_extract_archive(package, tmpdir);
    rmdirs(tmpdir);
    free(package);
    free(ucd);
}

int init_config_global() {
    SPM_GLOBAL.user_config_basedir = get_user_conf_dir();
    SPM_GLOBAL.user_config_file = get_user_config_file();
    SPM_GLOBAL.package_dir = realpath(PKG_DIR, NULL);
    if (SPM_GLOBAL.user_config_file) {
        SPM_GLOBAL.config = config_read(SPM_GLOBAL.user_config_file);
    }
}

void show_global_config() {
    printf("configuration directory: %s\n", SPM_GLOBAL.user_config_basedir);
    printf("configuration file: %s\n", SPM_GLOBAL.user_config_file);
    printf("configuration:\n");
    for (int i = 0; SPM_GLOBAL.config[i] != NULL; i++) {
        printf("-> %s\n", SPM_GLOBAL.config[i]);
    }
}

int main(int argc, char *argv[]) {
    // not much to see here yet
    // at the moment this will all be random tests, for better or worse
    // everything here is subject to change without notice

    // Initialize configuration data
    init_config_global();
    show_global_config();

    // Ensure external programs are available for use.
    check_runtime_environment();

    printf("find_package test:\n");
    const char *pkg_name = "python";
    char *package = NULL;
    package = find_package(pkg_name);

    if (package != NULL) {
        printf("Package found: %s\n", package);
        free(package);
    } else {
        fprintf(stderr, "Package does not exist: %s\n", pkg_name);
    }

    /*
    char **files = fstree("/bin");
    for (int i = 0; files[i] != NULL; i++) {
        if (fstrstr(files[i], "/usr/lib") == 0) {
            printf("%d: %s\n", i, files[i]);
        }
        free(files[i]);
    }
    free(files);
    */

    /*
    if (mkdirs("/tmp/this/is/a/test", 0755) == -1 && errno) {
        fprintf(SYSERROR);
    }
    */

    char *test_path = realpath("root/lib/python3.7/lib-dynload/_multiprocessing.cpython-37m-x86_64-linux-gnu.so", NULL);
    if (!test_path) {
        fprintf(stderr, "Unable to get absolute path for %s\n", test_path);
        exit(1);
    }
    char *rpath = gen_rpath(test_path);

    if (!rpath) {
        fprintf(stderr, "Unable to generate RPATH for %s\n", test_path);
        free(test_path);
        exit(1);
    }

    printf("Setting RPATH: %s\n", test_path);
    if (set_rpath(test_path, rpath) != 0) {
        fprintf(stderr, "RPATH assignment failed\n");
    }

    install("/tmp/root", "python");
    free(test_path);
    free(rpath);
    return 0;
}
