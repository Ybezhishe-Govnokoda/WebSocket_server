#define WS_DLL_EXPORTS
#include "../include/ws_api.h"
#include "ws_client.hpp"

extern "C" {

   ws_handle_t ws_create() {
      return new WsClient();
   }

   void ws_destroy(ws_handle_t h) {
      delete static_cast<WsClient *>(h);
   }

   int ws_connect(ws_handle_t h,
      const char *url,
      const char *token) {
      return static_cast<WsClient *>(h)->connect(url, token);
   }

   void ws_disconnect(ws_handle_t h) {
      static_cast<WsClient *>(h)->disconnect();
   }

   void ws_set_on_connected(ws_handle_t h, ws_on_connected_cb cb) {
      static_cast<WsClient *>(h)->on_connected =
         [cb](bool c) { if (cb) cb(c); };
   }

   void ws_set_on_message(ws_handle_t h, ws_on_message_cb cb) {
      static_cast<WsClient *>(h)->on_message =
         [cb](const std::string &msg) {
         if (cb) cb(msg.c_str());
         };
   }

   void ws_set_on_error(ws_handle_t h, ws_on_error_cb cb) {
      static_cast<WsClient *>(h)->on_error =
         [cb](int c, const std::string &t) {
         if (cb) cb(c, t.c_str());
         };
   }

}
