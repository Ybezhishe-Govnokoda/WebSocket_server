#include "ws_api.h"
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
   auto server = ws_server_create();

   ws_server_set_on_client_connected(server, on_connect);
   ws_server_set_on_client_disconnected(server, on_disconnect);
   ws_server_set_on_message(server, on_message);
   ws_server_set_on_error(server, on_error);

   ws_server_start(server, 9002);

   std::cout << "Server running. Press Enter to stop...\n";
   std::cin.get();

   ws_server_stop(server);
   ws_server_destroy(server);
}
