#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <functional>
#include <string>

class WsClient {
public:
   WsClient();
   ~WsClient();

   bool connect(const std::string &url, const std::string &token);
   void disconnect();

   /* callbacks (C++) */
   std::function<void(bool)> on_connected;
   std::function<void(const std::string &)> on_message;
   std::function<void(int, const std::string &)> on_error;

private:
   void read_loop();

   boost::asio::io_context ioc_;
   boost::asio::ip::tcp::resolver resolver_;
   boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;

   std::thread io_thread_;
   bool connected_{ false };
};
