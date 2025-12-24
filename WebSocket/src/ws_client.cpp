#include "ws_client.hpp"
#include <iostream>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;

WsClient::WsClient()
   : resolver_(ioc_), ws_(ioc_) {
}

WsClient::~WsClient() {
   disconnect();
}

bool WsClient::connect(const std::string &url,
   const std::string &token) {
   try {
      auto const pos = url.find(':');
      std::string host = url.substr(0, pos);
      std::string port = url.substr(pos + 1);

      auto results = resolver_.resolve(host, port);
      asio::connect(ws_.next_layer(), results);

      ws_.set_option(websocket::stream_base::decorator(
         [&](websocket::request_type &req) {
            req.set("Authorization", "Bearer " + token);
         }));

      ws_.handshake(host, "/");
      connected_ = true;

      if (on_connected) on_connected(true);

      io_thread_ = std::thread([this]() { read_loop(); });
      return true;
   }
   catch (const std::exception &e) {
      if (on_error) on_error(-1, e.what());
      return false;
   }
}

void WsClient::disconnect() {
   if (!connected_) return;

   try {
      ws_.close(websocket::close_code::normal);
   }
   catch (...) {}

   connected_ = false;
   if (on_connected) on_connected(false);
}

void WsClient::read_loop() {
   try {
      for (;;) {
         beast::flat_buffer buffer;
         ws_.read(buffer);

         if (on_message)
            on_message(beast::buffers_to_string(buffer.data()));
      }
   }
   catch (const std::exception &e) {
      if (on_error) on_error(-2, e.what());
   }
}
