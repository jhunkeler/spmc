/**
 * @file checksum.c
 */
#include "spm.h"
#include <openssl/md5.h>
#include <openssl/sha.h>

/**
 *
 * @param filename
 * @return
 */
char *md5sum(const char *filename) {
    size_t bytes = 0;
    unsigned char digest[MD5_DIGEST_LENGTH];
    char buf[BUFSIZ];
    MD5_CTX context;
    MD5_Init(&context);
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return NULL;
    }
    char *result = calloc((MD5_DIGEST_LENGTH * 2) + 1, sizeof(char));
    if (!result) {
        fclose(fp);
        perror("MD5 result");
        return NULL;
    }

    while ((bytes = fread(buf, sizeof(char), BUFSIZ, fp)) != 0) {
        MD5_Update(&context, buf, bytes);
    }
    fclose(fp);

    MD5_Final(digest, &context);
    char *rtmp = result;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        snprintf(&rtmp[i * 2], 3, "%02x", digest[i]);
    }

    return result;
}

/**
 *
 * @param filename
 * @return
 */
char *sha256sum(const char *filename) {
    size_t bytes = 0;
    unsigned char digest[SHA256_DIGEST_LENGTH];
    char buf[BUFSIZ];
    SHA256_CTX context;
    SHA256_Init(&context);
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return NULL;
    }
    char *result = calloc((SHA256_DIGEST_LENGTH * 2) + 1, sizeof(char));
    if (!result) {
        fclose(fp);
        perror("SHA256 result");
        return NULL;
    }

    while ((bytes = fread(buf, sizeof(char), BUFSIZ, fp)) != 0) {
        SHA256_Update(&context, buf, bytes);
    }
    fclose(fp);

    SHA256_Final(digest, &context);
    char *rtmp = result;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(&rtmp[i * 2], 3, "%02x", digest[i]);
    }

    return result;
}