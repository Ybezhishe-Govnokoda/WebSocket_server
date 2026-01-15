#include "wss_api.h"
#include <iostream>

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
    std::cout << "WSS Server" << std::endl;
    // wss init
    auto wss = wss_server_create();

    wss_server_set_on_client_connected(wss, on_connect);
    wss_server_set_on_client_disconnected(wss, on_disconnect);
    wss_server_set_on_message(wss, on_message);
    wss_server_set_on_error(wss, on_error);

    // SERVER START
    // 9002 - for wss://
    // 9003 - for ws://

    // wss://localhost:9002
    // ws://localhost:9003

    wss_server_start(wss, 9002);

    std::cin.get();

    wss_server_stop(wss);
    wss_server_destroy(wss);
}
