#include <crypto.h>
#include <mongoose/mongoose.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  int exit_code = 1;
  char *msg = "Cryptography fucking sucks";
  char *expected = "1Ct943GAT+6YQUUX47Ia/ncufilbe6+oD6lY+5kaCu4=";
  char *auth = NULL;

  // Test strings taken from the protocol document.
  create_auth_string("+IxH4CnCiqpX1rM9scsNynZzbOe4KhDeYcTNS3PDaeY=",
                     "lM1GncleQOaCu9lT1yeUZhFYnqhsLLP1G5lAGo3ixaI=",
                     "supersecretpassword", &auth);
  if (!auth) goto fail;
  if (strcmp(expected, auth) != 0) goto fail;

  msg = "Cryptography is easy peasy";
  exit_code = 0;
fail:
  printf(">>> %s\n", msg);
  if (auth) mg_free(auth);
  return exit_code;
}
