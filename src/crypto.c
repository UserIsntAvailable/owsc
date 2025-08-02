#include "crypto.h"
#include "external/mongoose.h"

void create_auth_string(const char *challenge, const char *salt,
                        const char *password, char **auth)
{
  if (!challenge || !salt || !password)
  {
    *auth = NULL;
    return;
  };

  mg_sha256_ctx ctx;
  char b64[45];
  uint8_t sha256[SHA256_BLOCK_SIZE];

  mg_sha256_init(&ctx);
  /* Concatenate the password with the salt provided by the server */
  mg_sha256_update(&ctx, (unsigned char *)password, strlen(password));
  mg_sha256_update(&ctx, (unsigned char *)salt, strlen(salt));

  /* Generate an SHA256 binary hash of the result and base64 encode it */
  mg_sha256_final(sha256, &ctx);
  size_t len = mg_base64_encode(sha256, sizeof(sha256), b64, sizeof(b64));

  mg_sha256_init(&ctx);
  /* Concatenate the base64 secret with the challenge sent by the server */
  mg_sha256_update(&ctx, (unsigned char *)b64, len);
  mg_sha256_update(&ctx, (unsigned char *)challenge, strlen(challenge));

  /* Generate a binary SHA256 hash of that result and base64 encode it */
  mg_sha256_final(sha256, &ctx);
  mg_base64_encode(sha256, sizeof(sha256), b64, sizeof(b64));

  *auth = mg_strdup(mg_str((char *)b64)).buf;
}
