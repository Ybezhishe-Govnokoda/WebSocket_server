#include "ws_api.h"
#include "wss_api.h"
#include <iostream>
#include <thread>

void on_connect(const char *id) {
    std::cout << "[CONNECT] " << id << "\n";
}

void on_disconnect(const char *id) {
    std::cout << "[DISCONNECT] " << id << "\n";
}

void on_message(const char *id, const char *msg) {
    std::cout << "[" << id << "] " << msg << "\n";
}

void on_error(int code, const char *text) {
    std::cout << "[ERROR] " << code << ": " << text << "\n";
}

int main() {
    // WS init
    auto ws = ws_server_create();
    ws_server_set_on_client_connected(ws, on_connect);
    ws_server_set_on_client_disconnected(ws, on_disconnect);
    ws_server_set_on_message(ws, on_message);
    ws_server_set_on_error(ws, on_error);

    // WSS init
    auto wss = wss_server_create();
    wss_server_set_on_client_connected(wss, on_connect);
    wss_server_set_on_client_disconnected(wss, on_disconnect);
    wss_server_set_on_message(wss, on_message);
    wss_server_set_on_error(wss, on_error);

    std::thread ws_thread([&] {
        ws_server_start(ws, 9003);
    });

    std::thread wss_thread([&] {
        wss_server_start(wss, 9002);
    });

    std::cin.get();

    // WS and WSS destroy
    ws_server_stop(ws);
    wss_server_stop(wss);

    ws_thread.join();
    wss_thread.join();

    ws_server_destroy(ws);
    wss_server_destroy(wss);
}
