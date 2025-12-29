#include "ws_api.h"
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
   ws_handle_t h = ws_client_create();

   ws_client_set_on_connected(h, on_connected);
   ws_client_set_on_message(h, on_message);
   ws_client_set_on_error(h, on_error);

   ws_client_connect(h, "localhost:9002", "test_token");

	std::cout << "Enter messages to send to the server. Ctrl+D to exit.\n";
   std::string line;
   while (std::getline(std::cin, line)) {
      ws_client_send(h, line.c_str());
   }

   ws_client_disconnect(h);
   ws_client_destroy(h);
}
