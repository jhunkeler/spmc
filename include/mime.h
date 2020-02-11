#ifndef SPM_MIME_H
#define SPM_MIME_H

typedef struct {
    char *origin;
    char *type;
    char *charset;
} Mime;

Process *file_command(const char *_filename);
Mime *file_mimetype(const char *filename);
void mime_free(Mime *m);
int build(int bargc, char **bargv);
int file_is_binary(const char *filename);
int file_is_text(const char *filename);
int file_is_binexec(const char *filename);

#endif //SPM_MIME_H
