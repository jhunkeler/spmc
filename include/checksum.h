/**
 * @file checksum.h
 */
#ifndef SPM_CHECKSUM_H
#define SPM_CHECKSUM_H

char *md5sum(const char *filename);
char *sha256sum(const char *filename);

#endif //SPM_CHECKSUM_H
