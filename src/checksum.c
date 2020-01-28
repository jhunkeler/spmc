/**
 * @file checksum.c
 */
#include "spm.h"
#include <openssl/sha.h>

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
    char *result = calloc(SHA256_DIGEST_STRING_LENGTH, sizeof(char));
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
