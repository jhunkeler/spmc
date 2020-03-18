/**
 * @file archive.h
 */
#ifndef SPM_ARCHIVE_H
#define SPM_ARCHIVE_H

int tar_extract_archive(const char *_archive, const char *_destination);
int tar_extract_file(const char *archive, const char* filename, const char *destination);

#endif //SPM_ARCHIVE_H
