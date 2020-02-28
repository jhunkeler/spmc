#ifndef SPM_METADATA_H
#define SPM_METADATA_H
#include "conf.h"

#define SPM_METADATA_VERIFY 0 << 1
#define SPM_METADATA_NO_VERIFY 1 << 1

typedef int (ReaderFn)(size_t line, char **);

char **spm_metadata_read(const char *filename, int no_verify);
int spm_metadata_remove(const char *_path);

// TODO: Can this somehow get hooked into spm_metadata_read()?
ConfigItem **spm_descriptor_read(const char *filename);

#endif //SPM_METADATA_H
