#ifndef OWSC_H
#define OWSC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mongoose/mongoose.h>

typedef struct owsc owsc;
typedef bool (*owsc_send_request_cb)(owsc *, struct mg_str, void *);

struct owsc {
  const char *host;
  const char *password; // gets set to `NULL` after identified.

  struct mg_mgr mgr;
  struct mg_connection *con;

  // XXX(Unavailable): Different error values.
  //
  // Use cases:
  //  - Timeout
  //  - Incorrect password
  //  - Server errors
  int status;
  bool identified;
  owsc_send_request_cb cb;
  void *cb_data;

  // Configuration

  // connection timeout (in ms).
  uint timeout;
  // set to a negative number to desactivate logging.
  int log_level;
  // https://github.com/obsproject/obs-websocket/blob/d957b27/docs/generated/protocol.md#eventsubscription
  uint subscribed_events;
};

void owsc_init(owsc *cln);

void owsc_free(owsc *cln);

int owsc_connect(owsc *cln, const char *host, const char *password);

int owsc_send_request(owsc *cln, const char *request_type,
                      owsc_send_request_cb cb, void *cb_data,
                      const char *format, ...);

int owsc_send_request_va(owsc *cln, const char *request_type,
                         owsc_send_request_cb cb, void *cb_data,
                         const char *format, va_list *ap);

int owsc_wait_for_response(owsc *cln);

// Helpers

// useful for `Toggle*` operations.
static inline int owsc_send_request_simple(owsc *cln,
                                           const char *request_type) {
  return owsc_send_request(cln, request_type, NULL, NULL, NULL);
}

static inline int owsc_send_request_no_cb(owsc *cln, const char *request_type,
                                          const char *format, ...) {
  va_list vl;
  va_start(vl, format);
  int ret = owsc_send_request_va(cln, request_type, NULL, NULL, format, &vl);
  va_end(vl);
  return ret;
}

#ifdef __cplusplus
}
#endif

#endif // OWSC_H
