#include <owsc.h>

// XXX(Unavailable): Take `HOST` and `PASSWORD` from env to make example easier
// to run?

bool cb(owsc *cln, struct mg_str json, void *data)
{
  return (*(char **)data =
            mg_json_get_str(json, "$.d.responseData.platformDescription"));
}

int main(void)
{
  int exit_code = 1;

  owsc cln = {.log_level = MG_LL_VERBOSE};
  owsc_init(&cln);
  // host defaults to `ws://localhost:4455`.
  if (owsc_connect(&cln, NULL, NULL) < 0) goto fail;

  char *desc;
  if (owsc_send_request(&cln, "GetVersion", cb, &desc, NULL) < 0) goto fail;
  printf(">>> Platform Description: %s\n", desc); // Arch Linux btw
  mg_free(desc);

  exit_code = 0;
fail:
  owsc_free(&cln);
  return exit_code;
}
