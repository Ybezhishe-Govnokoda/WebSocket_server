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
   auto s = ws_server_create();

   ws_server_set_on_client_connected(s, on_connect);
   ws_server_set_on_client_disconnected(s, on_disconnect);
   ws_server_set_on_message(s, on_message);
   ws_server_set_on_error(s, on_error);

   ws_server_start(s, 9002);

   std::cin.get();

   ws_server_stop(s);
   ws_server_destroy(s);
}
