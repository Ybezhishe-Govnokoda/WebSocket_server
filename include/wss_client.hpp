#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <thread>
#include <functional>
#include <string>

using tcp = boost::asio::ip::tcp;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = boost::asio::ssl;

struct ParsedUrl {
    std::string host;
    std::string port;
    std::string path;
};

class WssClient {
public:
    WssClient();
    ~WssClient();

    int connect(const std::string &url, const std::string &token);
    void disconnect();
    bool send(const std::string &msg);

    std::function<void(bool)> on_connected;
    std::function<void(const std::string &)> on_message;
    std::function<void(int, const std::string &)> on_error;

private:
    asio::io_context ioc_;
    asio::strand<asio::io_context::executor_type> strand_;
    tcp::resolver resolver_;

    ssl::context ssl_ctx_{ssl::context::tls_client};

    std::unique_ptr<websocket::stream<ssl::stream<tcp::socket>>> wss_;

    asio::steady_timer ping_timer_;
    asio::steady_timer pong_timeout_;
    asio::steady_timer reconnect_timer_;

    std::thread io_thread_;
    bool connected_{ false };

    std::string last_url_;
    std::string last_token_;
    int reconnect_delay_{ 1 };

    ParsedUrl parse_ws_url(const std::string& url, bool is_wss);
    void do_connect();
    void do_read();
    void start_ping();
    void schedule_reconnect();
};

