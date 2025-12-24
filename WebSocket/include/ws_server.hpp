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

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace beast = boost::beast;

class WsServer {
public:
   WsServer();
   ~WsServer();

   bool start(uint16_t port);
   void stop();

   /* callbacks */
   void set_on_client_connected(std::function<void(const std::string &)> cb);
   void set_on_client_disconnected(std::function<void(const std::string &)> cb);
   void set_on_message(std::function<void(const std::string &, const std::string &)> cb);
   void set_on_error(std::function<void(int, const std::string &)> cb);

private:
   struct Client {
      std::string id;
      websocket::stream<tcp::socket> ws;
      boost::beast::flat_buffer buffer;

      Client(tcp::socket socket, std::string cid);
   };

   boost::asio::io_context ioc_;
   std::unique_ptr<tcp::acceptor> acceptor_;
   std::thread io_thread_;

   std::mutex clients_mutex_;
   std::unordered_map<std::string, std::shared_ptr<Client>> clients_;

   /* callbacks */
   std::function<void(const std::string &)> on_client_connected_;
   std::function<void(const std::string &)> on_client_disconnected_;
   std::function<void(const std::string &, const std::string &)> on_message_;
   std::function<void(int, const std::string &)> on_error_;

   void do_accept();
   void do_read(std::shared_ptr<Client> client);
   void close_client(const std::string &id);
};
