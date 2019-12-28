/**
 * @file shell.c
 */
#include "spm.h"

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
