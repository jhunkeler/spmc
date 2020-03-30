/**
 * @file shell.h
 */
#ifndef SPM_SHELL_H
#define SPM_SHELL_H

#define SHELL_INVALID "&;|"
#define SHELL_DEFAULT 1 << 0
#define SHELL_OUTPUT 1 << 1
#define SHELL_BENCHMARK 1 << 2

typedef struct {
    double time_elapsed;
    int returncode;
    char *output;
} Process;

void shell(Process **proc_info, u_int64_t option, const char *fmt, ...);
void shell_free(Process *proc_info);

#endif //SPM_SHELL_H
