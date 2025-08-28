#ifndef LCHK_CRYPTO_H
#define LCHK_CRYPTO_H

/* Reads secret from specified location. */
char *lchk_read_secret(const char *path);

/* Signs 'data' using HMAC method with secret 'key'. */
char *lchk_hmac_sign(const char *data, const char *key);

#endif /* LCHK_CRYPTO_H */
