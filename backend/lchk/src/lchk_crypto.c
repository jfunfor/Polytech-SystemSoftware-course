#include "lchk.h"

char *lchk_read_secret(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("[ERROR] Failed to open HMAC secret file");
        return NULL;
    }

    char *secret = malloc(LCHK_MAX_SECRET_LEN);
    if (!secret) {
        fclose(f);
        fprintf(stderr, "[ERROR] Memory allocation failed\n");
        return NULL;
    }

    if (!fgets(secret, LCHK_MAX_SECRET_LEN, f)) {
        fclose(f);
        free(secret);
        fprintf(stderr, "[ERROR] Failed to read secret\n");
        return NULL;
    }

    fclose(f);

    /* Strip trailing newline */
    secret[strcspn(secret, "\n")] = 0;
    return secret;
}

char *lchk_hmac_sign(const char *data, const char *key) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    HMAC(EVP_sha256(), key, strlen(key), (unsigned char *)data, strlen(data), digest, &digest_len);

    /* Convert to hex string */
    char *hex = malloc(digest_len * 2 + 1);
    for (unsigned int i = 0; i < digest_len; i++)
        sprintf(&hex[i * 2], "%02x", digest[i]);

    return hex;
}
