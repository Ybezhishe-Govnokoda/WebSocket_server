#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>

using tcp = boost::asio::ip::tcp;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;

class WsServer {
public:
   WsServer();
   ~WsServer();

   int start(int port);
   void stop();

   /* callbacks */
   void set_on_client_connected(std::function<void(const std::string &)> cb) {
      on_client_connected_ = std::move(cb);
   }
   void set_on_client_disconnected(std::function<void(const std::string &)> cb) {
      on_client_disconnected_ = std::move(cb);
   }
   void set_on_message(
      std::function<void(const std::string &, const std::string &)> cb) {
      on_message_ = std::move(cb);
   }
   void set_on_error(std::function<void(int, const std::string &)> cb) {
      on_error_ = std::move(cb);
   }


private:
   struct Client {
      std::string id;
      websocket::stream<tcp::socket> ws;
      beast::flat_buffer buffer;
      asio::steady_timer ping_timer;

      explicit Client(tcp::socket socket);
   };

   asio::io_context ioc_;
   tcp::acceptor acceptor_;
   std::thread io_thread_;
   std::atomic<bool> running_{ false };

   std::mutex clients_mutex_;
   std::unordered_map<std::string, std::shared_ptr<Client>> clients_;

   std::function<void(const std::string &)> on_client_connected_;
   std::function<void(const std::string &)> on_client_disconnected_;
   std::function<void(const std::string &, const std::string &)> on_message_;
   std::function<void(int, const std::string &)> on_error_;

   void do_accept();
   void accept_ws(std::shared_ptr<Client> client);
   void do_read(std::shared_ptr<Client> client);
   void start_ping(std::shared_ptr<Client> client);
   void handle_disconnect(std::shared_ptr<Client> client, beast::error_code ec);
};
