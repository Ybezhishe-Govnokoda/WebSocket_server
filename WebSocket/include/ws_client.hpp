#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <functional>
#include <string>

using tcp = boost::asio::ip::tcp;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;

class WsClient {
public:
   WsClient();
   ~WsClient();

   int connect(const std::string &url, const std::string &token);
   void disconnect();

   std::function<void(bool)> on_connected;
   std::function<void(const std::string &)> on_message;
   std::function<void(int, const std::string &)> on_error;

private:
   asio::io_context ioc_;
   tcp::resolver resolver_;
   websocket::stream<tcp::socket> ws_;
   asio::steady_timer ping_timer_;
   asio::steady_timer pong_timeout_;

   std::thread io_thread_;
   bool connected_{ false };

   std::string last_url_;
   std::string last_token_;
   int reconnect_delay_{ 1 };

   void do_connect();
   void do_read();
   void start_ping();
   void schedule_reconnect();
};
