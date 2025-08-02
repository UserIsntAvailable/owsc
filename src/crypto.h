#ifndef CRYPTO_H
#define CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

#define SHA256_BLOCK_SIZE 32

void create_auth_string(const char *challenge, const char *salt,
                        const char *password, char **auth);

#ifdef __cplusplus
}
#endif
#endif // CRYPTO_H
