#include "ws_client.hpp"
#include <iostream>

WsClient::WsClient()
   : resolver_(ioc_),
   ws_(ioc_),
   ping_timer_(ioc_),
   pong_timeout_(ioc_) {
}

WsClient::~WsClient() {
   disconnect();
}

int WsClient::connect(const std::string &url, const std::string &token) {
   last_url_ = url;
   last_token_ = token;

   do_connect();
   io_thread_ = std::thread([this] { ioc_.run(); });
   return 0;
}

void WsClient::do_connect() {
   auto pos = last_url_.find(':');
   auto host = last_url_.substr(0, pos);
   auto port = last_url_.substr(pos + 1);

   resolver_.async_resolve(host, port,
      [this, host](auto ec, auto results) {
         if (ec) return schedule_reconnect();

         asio::async_connect(ws_.next_layer(), results,
            [this, host](auto ec, auto) {
               if (ec) return schedule_reconnect();

               ws_.set_option(websocket::stream_base::decorator(
                  [this](auto &req) {
                     req.set("Authorization", "Bearer " + last_token_);
                  }));

               ws_.async_handshake(host, "/",
                  [this](auto ec) {
                     if (ec) return schedule_reconnect();

                     connected_ = true;
                     reconnect_delay_ = 1;
                     if (on_connected) on_connected(true);

                     ws_.control_callback(
                        [this](websocket::frame_type type, auto) {
                           if (type == websocket::frame_type::pong)
                              pong_timeout_.cancel();
                        });

                     start_ping();
                     do_read();
                  });
            });
      });
}

void WsClient::start_ping() {
   ping_timer_.expires_after(std::chrono::seconds(5));
   ping_timer_.async_wait([this](auto ec) {
      if (ec) return;

      ws_.async_ping({}, [this](auto ec) {
         if (ec) return schedule_reconnect();
         });

      pong_timeout_.expires_after(std::chrono::seconds(3));
      pong_timeout_.async_wait([this](auto ec) {
         if (!ec) schedule_reconnect();
         });

      start_ping();
      });
}

void WsClient::do_read() {
   auto buffer = std::make_shared<beast::flat_buffer>();
   ws_.async_read(*buffer,
      [this, buffer](auto ec, auto bytes) {
         if (ec) return schedule_reconnect();

         if (on_message)
            on_message(beast::buffers_to_string(buffer->data()));

         do_read();
      });
}

void WsClient::schedule_reconnect() {
   if (connected_) {
      connected_ = false;
      if (on_connected) on_connected(false);
   }

   ping_timer_.cancel();
   pong_timeout_.cancel();

   asio::steady_timer t(ioc_);
   t.expires_after(std::chrono::seconds(reconnect_delay_));
   t.wait();

   reconnect_delay_ = std::min(reconnect_delay_ * 2, 10);
   do_connect();
}

void WsClient::disconnect() {
   asio::post(ioc_, [this] {
      beast::error_code ec;
      ws_.close(websocket::close_code::normal, ec);
      });

   ioc_.stop();
   if (io_thread_.joinable())
      io_thread_.join();
}
