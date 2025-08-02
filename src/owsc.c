#include "owsc.h"
#include "crypto.h"

// XXX(Unavailable): More `MG_DEBUG` logs.
// XXX(Unavailable): Allow `tls` connections.
// FIXME(Unavailable): I'm not checking if writing to the server ever fails.

static int handle_op_code_zero(owsc *cln, struct mg_str msg)
{
  int success = -1;

  if (cln->identified)
  {
    MG_ERROR(
      ("The server sended `Identify`, but this client is already identified"));
    return -1;
  }

  char *challenge = mg_json_get_str(msg, "$.d.authentication.challenge");
  char *salt = mg_json_get_str(msg, "$.d.authentication.salt");
  char *auth = NULL;

  if (!challenge && !salt)
  {
    mg_ws_printf(cln->con, WEBSOCKET_OP_TEXT, "{%m:%d,%m:{%m:%d,%m:%d}}",
                 MG_ESC("op"), 1, MG_ESC("d"), MG_ESC("rpcVersion"), 1,
                 MG_ESC("eventSubscriptions"), cln->subscribed_events);
    return 0;
  }
  if (!(challenge && salt))
  {
    MG_ERROR(
      ("Invalid server response; missing `d.authentication.challenge` or "
       "`d.authentication.salt` keys"));
    goto fail;
  }

  create_auth_string(challenge, salt, cln->password, &auth);
  if (!auth)
  {
    MG_ERROR(("The server sended an invalid `challenge` and `salt` key"));
    goto fail;
  }

  mg_ws_printf(cln->con, WEBSOCKET_OP_TEXT, "{%m:%d,%m:{%m:%d,%m:%m,%m:%d}}",
               MG_ESC("op"), 1, MG_ESC("d"), MG_ESC("rpcVersion"), 1,
               MG_ESC("authentication"), MG_ESC(auth),
               MG_ESC("eventSubscriptions"), cln->subscribed_events);
  success = 0;
fail:
  if (challenge) mg_free(challenge);
  if (salt) mg_free(salt);
  if (auth) mg_free(auth);
  return success;
}

static int handle_op_code_two(owsc *cln, struct mg_str msg)
{
  long negocitated_rpc_ver;
  if ((negocitated_rpc_ver =
         mg_json_get_long(msg, "$.d.negotiatedRpcVersion", -1)) == -1)
  {
    MG_ERROR(("Invalid server response; missing `d.negotiatedRpcVersion` key"));
    return -1;
  }

  if (negocitated_rpc_ver != 1)
  {
    MG_ERROR(("The obs websocket server requires `rpcVersion` of %i",
              negocitated_rpc_ver));
    return -1;
  };

  cln->identified = true;
  cln->password = NULL;

  return 0;
}

static int handle_response(owsc *cln, struct mg_str msg)
{
  long op_code;
  if ((op_code = mg_json_get_long(msg, "$.op", -1)) == -1)
  {
    MG_ERROR(("Invalid server response; missing `op` key"));
    return -1;
  }

  switch (op_code)
  {
    // server op codes
    case 0: // Hello
    {
      int ret = handle_op_code_zero(cln, msg);
      return ret < 0 ? ret : 1; // wait for server response.
    }
    case 2: // Identified
      return handle_op_code_two(cln, msg);
    case 5:
      // XXX(Unavailable): Implement event streaming.
      MG_DEBUG(("Event streaming is currently not supported"));
      return 0;
    case 7:
    case 9:
      // FIXME(Unavailable): Check response code.
      if (cln->cb) return cln->cb(cln, msg, cln->cb_data) == true ? 0 : -1;
      break;
    // client op codes
    case 1:
    case 3:
    case 6:
    case 8:
      MG_ERROR(("The server has responsed with an `op` code (%i) which is "
                "reserved for clients",
                op_code));
      return -1;
    default:
      MG_DEBUG(
        ("Got unknown `op` code (%i) from the server (ignored)", op_code));
  };

  return 0;
}

static void event_handler(struct mg_connection *con, int event, void *data)
{
  owsc *cln = con->fn_data;

  switch (event)
  {
    case MG_EV_OPEN:
      *(uint64_t *)con->data = mg_millis() + cln->timeout;
      if (cln->log_level == MG_LL_VERBOSE) con->is_hexdumping = true;
      break;
    case MG_EV_WS_OPEN:
      MG_DEBUG(("Connected to %s", cln->host));
      break;
    case MG_EV_CLOSE:
      cln->status = 0;
      MG_DEBUG(("Closed connection with %s", cln->host));
      break;
    case MG_EV_POLL:
      if (mg_millis() > *(uint64_t *)con->data &&
          (con->is_connecting || con->is_resolving))
      {
        cln->status = -1;
        MG_ERROR(("Conection with server timeout"));
      }
      break;
    case MG_EV_WS_MSG:
    {
      struct mg_ws_message *wm = data;
      cln->status = handle_response(cln, wm->data);
      break;
    }
    case MG_EV_WS_CTL:
    {
      struct mg_ws_message *wm = data;
      if ((wm->flags & 15) == WEBSOCKET_OP_CLOSE)
      {
        cln->status = -1;
        struct mg_str msg = wm->data;
        if (msg.len < sizeof(uint16_t))
          MG_ERROR(("Connection closed without body"));
        else
          MG_ERROR(("Connection closed with code (%d): %.*s",
                    MG_LOAD_BE16(msg.buf), msg.len - sizeof(uint16_t),
                    msg.buf + sizeof(uint16_t)));
      }
      break;
    }
    case MG_EV_ERROR:
      cln->status = -1;
      MG_ERROR(("%p %s", con->fd, (char *)data));
      break;
  }
}

// Public API

void owsc_init(owsc *cln)
{
  mg_mgr_init(&cln->mgr);

  cln->status = 0;
  cln->timeout = 5000;

  if (cln->log_level == 0) cln->log_level = MG_LL_ERROR;
  if (cln->log_level > 0) mg_log_set(cln->log_level);

  cln->identified = false;
  cln->subscribed_events = 0;
}

void owsc_free(owsc *cln) { mg_mgr_free(&cln->mgr); }

int owsc_connect(owsc *cln, const char *host, const char *password)
{
  if (!host) host = "ws://localhost:4455";

  cln->host = host;
  cln->password = password;
  cln->con = mg_ws_connect(&cln->mgr, host, event_handler, cln, NULL);

  return owsc_wait_for_response(cln);
}

int owsc_send_request(owsc *cln, const char *request_type,
                      owsc_send_request_cb cb, void *cb_data,
                      const char *format, ...)
{
  va_list vl;
  va_start(vl, format);
  int ret = owsc_send_request_va(cln, request_type, cb, cb_data, format, &vl);
  va_end(vl);
  return ret;
}

int owsc_send_request_va(owsc *cln, const char *request_type,
                         owsc_send_request_cb cb, void *cb_data,
                         const char *format, va_list *ap)
{
  if (!cln->identified)
  {
    MG_ERROR(
      ("You need to first identify the client with the obs server by calling "
       "`owsc_connect()`, before being able to send a request"));
    return -1;
  }

  cln->cb = cb;
  cln->cb_data = cb_data;

  size_t len = cln->con->send.len;
  mg_xprintf(mg_pfn_iobuf, &cln->con->send, "{%m:%d,%m:{%m:%m,%m:%m",
             MG_ESC("op"), 6, MG_ESC("d"), MG_ESC("requestType"),
             MG_ESC(request_type), MG_ESC("requestId"),
             // XXX(Unavailable): I can keep this hardcoded, since the
             // library doesn't support multi request.
             MG_ESC("f819dcf0-89cc-11eb-8f0e-382c4ac93b9c"));
  if (format)
  {
    mg_xprintf(mg_pfn_iobuf, &cln->con->send, ",%m:", MG_ESC("requestData"));
    mg_vxprintf(mg_pfn_iobuf, &cln->con->send, format, ap);
  }
  mg_xprintf(mg_pfn_iobuf, &cln->con->send, "}}");
  mg_ws_wrap(cln->con, cln->con->send.len - len, WEBSOCKET_OP_TEXT);

  return owsc_wait_for_response(cln);
}

int owsc_wait_for_response(owsc *cln)
{
  cln->status = 1;
  while (cln->con && cln->status > 0)
    mg_mgr_poll(&cln->mgr, 1000);
  return cln->status;
}
