#include "ws_server.hpp"

#include <iostream>
#include <atomic>

static std::atomic_uint64_t g_client_counter{ 0 };

WsServer::Client::Client(tcp::socket socket, std::string cid)
   : id(std::move(cid))
   , ws(std::move(socket))
{
}

WsServer::WsServer() = default;

WsServer::~WsServer() {
   stop();
}

bool WsServer::start(uint16_t port) {
   try {
      acceptor_ = std::make_unique<tcp::acceptor>(
         ioc_, tcp::endpoint(tcp::v4(), port));

      do_accept();

      io_thread_ = std::thread([this]() {
         ioc_.run();
         });

      return true;
   }
   catch (const std::exception &e) {
      if (on_error_) {
         on_error_(-1, e.what());
      }
      return false;
   }
}

void WsServer::stop() {
   ioc_.stop();

   if (io_thread_.joinable()) {
      io_thread_.join();
   }

   std::lock_guard lock(clients_mutex_);
   clients_.clear();
}

void WsServer::set_on_client_connected(std::function<void(const std::string &)> cb) {
   on_client_connected_ = std::move(cb);
}

void WsServer::set_on_client_disconnected(std::function<void(const std::string &)> cb) {
   on_client_disconnected_ = std::move(cb);
}

void WsServer::set_on_message(std::function<void(const std::string &, const std::string &)> cb) {
   on_message_ = std::move(cb);
}

void WsServer::set_on_error(std::function<void(int, const std::string &)> cb) {
   on_error_ = std::move(cb);
}

void WsServer::do_accept() {
   acceptor_->async_accept(
      [this](beast::error_code ec, tcp::socket socket) {
         if (ec) {
            if (on_error_) {
               on_error_(ec.value(), ec.message());
            }
            return;
         }

         const std::string client_id =
            "client_" + std::to_string(++g_client_counter);

         auto client = std::make_shared<Client>(std::move(socket), client_id);

         client->ws.accept();

         {
            std::lock_guard lock(clients_mutex_);
            clients_[client_id] = client;
         }

         if (on_client_connected_) {
            on_client_connected_(client_id);
         }

         do_read(client);
         do_accept();
      });
}

void WsServer::do_read(std::shared_ptr<Client> client) {
   client->ws.async_read(
      client->buffer,
      [this, client](beast::error_code ec, std::size_t bytes) {
         if (ec) {
            close_client(client->id);
            return;
         }

         std::string msg = beast::buffers_to_string(client->buffer.data());
         client->buffer.consume(bytes);

         if (on_message_) {
            on_message_(client->id, msg);
         }

         /* echo back */
         client->ws.text(true);
         client->ws.async_write(
            boost::asio::buffer(msg),
            [](beast::error_code, std::size_t) {});

         do_read(client);
      });
}

void WsServer::close_client(const std::string &id) {
   std::lock_guard lock(clients_mutex_);

   auto it = clients_.find(id);
   if (it != clients_.end()) {
      if (on_client_disconnected_) {
         on_client_disconnected_(id);
      }
      clients_.erase(it);
   }
}
