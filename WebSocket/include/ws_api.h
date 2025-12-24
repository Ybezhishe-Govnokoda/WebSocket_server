#pragma once

#ifdef WS_DLL_EXPORTS
#define WS_API __declspec(dllexport)
#else
#define WS_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

   typedef void *ws_handle_t;

   /* callbacks */
   typedef void(*ws_on_connected_cb)(int connected);
   typedef void(*ws_on_message_cb)(const char *msg);
   typedef void(*ws_on_error_cb)(int code, const char *text);

   /* lifecycle */
   WS_API ws_handle_t ws_create();
   WS_API void ws_destroy(ws_handle_t h);

   /* connection */
   WS_API int  ws_connect(ws_handle_t h,
      const char *url,
      const char *bearer_token);
   WS_API void ws_disconnect(ws_handle_t h);

   /* callbacks registration */
   WS_API void ws_set_on_connected(ws_handle_t h, ws_on_connected_cb cb);
   WS_API void ws_set_on_message(ws_handle_t h, ws_on_message_cb cb);
   WS_API void ws_set_on_error(ws_handle_t h, ws_on_error_cb cb);

#ifdef __cplusplus
}
#endif
