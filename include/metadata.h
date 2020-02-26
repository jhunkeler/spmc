#ifndef SPM_METADATA_H
#define SPM_METADATA_H

#define SPM_METADATA_VERIFY 0 << 1
#define SPM_METADATA_NO_VERIFY 1 << 1

typedef int (ReaderFn)(size_t line, char **);

char **metadata_filelist_read(const char *filename);
char **metadata_read(const char *filename, int no_verify);

#endif //SPM_METADATA_H
