#include "ws_api.h"
#include <iostream>

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
   ws_handle_t h = ws_create();

   ws_set_on_connected(h, on_connected);
   ws_set_on_message(h, on_message);
   ws_set_on_error(h, on_error);

   ws_connect(h, "localhost:9002", "test_token");

   std::cin.get();

   ws_disconnect(h);
   ws_destroy(h);
}
