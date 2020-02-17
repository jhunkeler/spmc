#ifndef SPM_RELOCATION_H
#define SPM_RELOCATION_H

#define PREFIX_WRITE_BIN 0
#define PREFIX_WRITE_TEXT 1

typedef struct {
    char *prefix;
    char *path;
} RelocationEntry;

int relocate(const char *filename, const char *_oldstr, const char *_newstr);
void relocate_root(const char *destroot, const char *baseroot);
int replace_text(char *data, const char *_spattern, const char *_sreplacement);
int file_replace_text(char *filename, const char *spattern, const char *sreplacement);
RelocationEntry **prefixes_read(const char *filename);
void prefixes_free(RelocationEntry **entry);
int prefixes_write(const char *output_file, int mode, char **prefix, const char *tree);
int file_is_metadata(const char *path);

#endif //SPM_RELOCATION_H
