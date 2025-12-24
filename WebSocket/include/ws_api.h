#pragma once

#ifdef WS_DLL_EXPORTS
#define WS_API __declspec(dllexport)
#else
#define WS_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

   /* ========= CLIENT =========*/

   typedef void *ws_handle_t;

   /* callbacks */
   typedef void(*ws_on_connected_cb)(int connected);
   typedef void(*ws_on_message_cb)(const char *msg);
   typedef void(*ws_on_error_cb)(int code, const char *text);

   /* lifecycle */
   WS_API ws_handle_t ws_client_create();
   WS_API void ws_client_destroy(ws_handle_t h);

   /* connection */
   WS_API int  ws_client_connect(ws_handle_t h,
      const char *url,
      const char *bearer_token);
   WS_API void ws_client_disconnect(ws_handle_t h);

   /* callbacks registration */
   WS_API void ws_client_set_on_connected(ws_handle_t h, ws_on_connected_cb cb);
   WS_API void ws_client_set_on_message(ws_handle_t h, ws_on_message_cb cb);
   WS_API void ws_client_set_on_error(ws_handle_t h, ws_on_error_cb cb);


   /* ========= SERVER =========*/

   typedef void *ws_handle_t;
   typedef void *ws_server_handle_t;

   /* callbacks */
   typedef void(*ws_server_on_client_connected_cb)(const char *client_id);
   typedef void(*ws_server_on_client_disconnected_cb)(const char *client_id);
   typedef void(*ws_server_on_message_cb)(const char *client_id, const char *msg);
   typedef void(*ws_server_on_error_cb)(int code, const char *text);

   /* lifecycle */
   WS_API ws_server_handle_t ws_server_create();
   WS_API void ws_server_destroy(ws_server_handle_t h);

	/* server control */
   WS_API int  ws_server_start(ws_server_handle_t h, int port);
   WS_API void ws_server_stop(ws_server_handle_t h);

	/* callbacks registration */
   WS_API void ws_server_set_on_client_connected(ws_server_handle_t h, ws_server_on_client_connected_cb cb);
   WS_API void ws_server_set_on_client_disconnected(ws_server_handle_t h, ws_server_on_client_disconnected_cb cb);
   WS_API void ws_server_set_on_message(ws_server_handle_t h, ws_server_on_message_cb cb);
   WS_API void ws_server_set_on_error(ws_server_handle_t h, ws_server_on_error_cb cb);

#ifdef __cplusplus
}
#endif
