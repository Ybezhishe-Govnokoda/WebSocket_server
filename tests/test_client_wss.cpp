#include "wss_api.h"
#include <iostream>
#include <string>

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
    std::cout << "Enter url (wss://localhost:9002)" << std::endl;

    ws_handle_t h = wss_client_create();

    wss_client_set_on_connected(h, on_connected);
    wss_client_set_on_message(h, on_message);
    wss_client_set_on_error(h, on_error);

    std::string url;
    std::getline(std::cin, url);
    wss_client_connect(h, url.c_str(), "test_token");

    std::cout << "Enter messages to send to the server. Ctrl+D to exit.\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        wss_client_send(h, line.c_str());
    }

    wss_client_disconnect(h);
    wss_client_destroy(h);
}
