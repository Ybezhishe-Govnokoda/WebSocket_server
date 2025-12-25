#include "ws_server.hpp"
#include <iostream>

WsServer::Client::Client(tcp::socket socket)
   : ws(std::move(socket)),
   ping_timer(ws.get_executor()) {
}

WsServer::WsServer()
   : acceptor_(ioc_) {
}

WsServer::~WsServer() {
   stop();
}

int WsServer::start(int port) {
   try {
      tcp::endpoint ep(tcp::v4(), static_cast<uint16_t>(port));
      acceptor_.open(ep.protocol());
      acceptor_.set_option(asio::socket_base::reuse_address(true));
      acceptor_.bind(ep);
      acceptor_.listen();

      running_ = true;
      do_accept();

      io_thread_ = std::thread([this] { ioc_.run(); });
      return 0;
   }
   catch (const std::exception &e) {
      if (on_error_) on_error_(-1, e.what());
      return -1;
   }
}

void WsServer::stop() {
   if (!running_) return;
   running_ = false;

   asio::post(ioc_, [this] {
      acceptor_.close();
      clients_.clear();
      });

   ioc_.stop();
   if (io_thread_.joinable())
      io_thread_.join();
}

void WsServer::do_accept() {
   acceptor_.async_accept([this](beast::error_code ec, tcp::socket socket) {
      if (!ec) {
         auto client = std::make_shared<Client>(std::move(socket));
         client->id = std::to_string(reinterpret_cast<uintptr_t>(client.get()));

         {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_[client->id] = client;
         }

         accept_ws(client);
      }
      if (running_) do_accept();
      });
}

void WsServer::accept_ws(std::shared_ptr<Client> client) {
   client->ws.set_option(websocket::stream_base::timeout::suggested(
      beast::role_type::server));

   client->ws.async_accept([this, client](beast::error_code ec) {
      if (ec) return handle_disconnect(client, ec);

      if (on_client_connected_) on_client_connected_(client->id);
      start_ping(client);
      do_read(client);
      });
}

void WsServer::do_read(std::shared_ptr<Client> client) {
   client->ws.async_read(client->buffer,
      [this, client](beast::error_code ec, std::size_t bytes) {
         if (ec) {
            handle_disconnect(client, ec);
            return;
         }

         std::string msg = beast::buffers_to_string(client->buffer.data());
         client->buffer.consume(bytes);

         if (on_message_)
            on_message_(client->id, msg);

			// Echo the message back to the client
         auto out = std::make_shared<std::string>(msg);

         client->ws.text(true);
         client->ws.async_write(
            boost::asio::buffer(*out),
            [out](beast::error_code, std::size_t) {
            });

         do_read(client);
      });
}

void WsServer::start_ping(std::shared_ptr<Client> client) {
   client->ping_timer.expires_after(std::chrono::seconds(5));
   client->ping_timer.async_wait([this, client](beast::error_code ec) {
      if (ec) return;

      client->ws.async_ping({}, [this, client](beast::error_code ec) {
         if (ec) handle_disconnect(client, ec);
         });

      start_ping(client);
      });
}

void WsServer::handle_disconnect(std::shared_ptr<Client> client, beast::error_code ec) {
   if (on_client_disconnected_)
      on_client_disconnected_(client->id);

   std::lock_guard<std::mutex> lock(clients_mutex_);
   clients_.erase(client->id);
}
