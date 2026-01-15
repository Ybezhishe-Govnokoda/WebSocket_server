//#define WS_DLL_EXPORTS
#include "wss_api.h"
#include "wss_client.hpp"
#include "wss_server.hpp"

extern "C" {

// Client

ws_handle_t wss_client_create() {
    return new WssClient();
}

WS_API void wss_client_destroy(ws_handle_t h) {
    delete static_cast<WssClient *>(h);
}

WS_API int wss_client_connect(ws_handle_t h,
                             const char *url,
                             const char *token) {
    return static_cast<WssClient *>(h)->connect(url, token);
}

WS_API void wss_client_disconnect(ws_handle_t h) {
    static_cast<WssClient *>(h)->disconnect();
}

WS_API int wss_client_send(ws_handle_t h, const char *msg) {
    return static_cast<WssClient *>(h)->send(msg) ? 0 : -1;
}

WS_API void wss_client_set_on_connected(ws_handle_t h, ws_on_connected_cb cb) {
    static_cast<WssClient *>(h)->on_connected =
        [cb](bool c) { if (cb) cb(c); };
}

WS_API void wss_client_set_on_message(ws_handle_t h, ws_on_message_cb cb) {
    static_cast<WssClient *>(h)->on_message =
        [cb](const std::string &msg) {
            if (cb) cb(msg.c_str());
        };
}

WS_API void wss_client_set_on_error(ws_handle_t h, ws_on_error_cb cb) {
    static_cast<WssClient *>(h)->on_error =
        [cb](int c, const std::string &t) {
            if (cb) cb(c, t.c_str());
        };
}

// ==============
// WSS SERVER API
// ==============
struct wss_server_wrapper {
    WssServer server;

    ws_server_on_client_connected_cb on_connect = nullptr;
    ws_server_on_client_disconnected_cb on_disconnect = nullptr;
    ws_server_on_message_cb on_message = nullptr;
    ws_server_on_error_cb on_error = nullptr;
};

WS_API ws_server_handle_t wss_server_create() {
    return new wss_server_wrapper{};
}

WS_API void wss_server_destroy(ws_server_handle_t h) {
    delete static_cast<wss_server_wrapper *>(h);
}

WS_API int wss_server_start(ws_server_handle_t h, int port_wss) {
    auto *w = static_cast<wss_server_wrapper *>(h);

    w->server.set_on_client_connected([w](const std::string &id) {
        if (w->on_connect) w->on_connect(id.c_str());
    });

    w->server.set_on_client_disconnected([w](const std::string &id) {
        if (w->on_disconnect) w->on_disconnect(id.c_str());
    });

    w->server.set_on_message([w](const std::string &id, const std::string &msg) {
        if (w->on_message) w->on_message(id.c_str(), msg.c_str());
    });

    w->server.set_on_error([w](int code, const std::string &text) {
        if (w->on_error) w->on_error(code, text.c_str());
    });

    return w->server.start(static_cast<uint16_t>(port_wss)) == 0 ? 0 : -1;
}

WS_API void wss_server_stop(ws_server_handle_t h) {
    static_cast<wss_server_wrapper *>(h)->server.stop();
}

WS_API void wss_server_set_on_client_connected(ws_server_handle_t h, ws_server_on_client_connected_cb cb) {
    static_cast<wss_server_wrapper *>(h)->on_connect = cb;
}

WS_API void wss_server_set_on_client_disconnected(ws_server_handle_t h, ws_server_on_client_disconnected_cb cb) {
    static_cast<wss_server_wrapper *>(h)->on_disconnect = cb;
}

WS_API void wss_server_set_on_message(ws_server_handle_t h, ws_server_on_message_cb cb) {
    static_cast<wss_server_wrapper *>(h)->on_message = cb;
}

WS_API void wss_server_set_on_error(ws_server_handle_t h, ws_server_on_error_cb cb) {
    static_cast<wss_server_wrapper *>(h)->on_error = cb;
}

} // extern "C"
