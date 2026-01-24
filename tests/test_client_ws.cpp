#include "ws_api.h"
#include "wss_api.h"
#include <iostream>

enum WsMode {
  WS,
  WSS
};

void on_connected(int c) {
    std::cout << "Connected: " << c << "\n";
}

void on_message(const char *msg) {
    std::cout << "Message: " << msg << "\n";
}

void on_error(int code, const char *text) {
    std::cout << "Error " << code << ": " << text << "\n";
}

int main() {
    WsMode mode;
    ws_handle_t h;

    // Needed url for ws - localhost:9003
    // Needed url for ws - wss://localhost:9002
    std::cout << "Enter url (ws://localhost:9003 or wss://localhost:9002)" << std::endl;
    std::string url;
    std::string last_url;
    std::getline(std::cin, url);

    if (url.rfind("wss://", 0) == 0) {
        mode = WSS;
        last_url = url.substr(6);
        std::cout << "You choosed WSS" << std::endl;
    } else if (url.rfind("ws://", 0) == 0) {
        mode = WS;
        last_url = url.substr(5);
    } else {
        std::cout << "Invalid URL scheme (use ws:// or wss://)" << std::endl;
    }

    if (mode == WS) {
        h = ws_client_create();

        ws_client_set_on_connected(h, on_connected);
        ws_client_set_on_message(h, on_message);
        ws_client_set_on_error(h, on_error);

        ws_client_connect(h, last_url.c_str(), "test_token");

        std::cout << "Enter messages to send to the server. Ctrl+D to exit.\n";
        std::string line;
        while (std::getline(std::cin, line)) {
            ws_client_send(h, line.c_str());
        }

        ws_client_disconnect(h);
        ws_client_destroy(h);
    } else {
        std::cout << "Connecting with WSS" << std::endl;
        h = wss_client_create();

        wss_client_set_on_connected(h, on_connected);
        wss_client_set_on_message(h, on_message);
        wss_client_set_on_error(h, on_error);

        wss_client_connect(h, last_url.c_str(), "test_token");

        std::cout << "Enter messages to send to the server. Ctrl+D to exit.\n";
        std::string line;
        while (std::getline(std::cin, line)) {
            wss_client_send(h, line.c_str());
        }

        wss_client_disconnect(h);
        wss_client_destroy(h);
    }
}
