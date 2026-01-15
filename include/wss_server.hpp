#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <deque>

using tcp = boost::asio::ip::tcp;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = boost::asio::ssl;


// ========================
// CLASS FOR WSs CONNECTION
// ========================
class WssServer {
public:
    WssServer();
    ~WssServer();

    int start(int port_wss);
    void stop();

    /* callbacks */
    void set_on_client_connected(std::function<void(const std::string&)> cb);
    void set_on_client_disconnected(std::function<void(const std::string&)> cb);
    void set_on_message(std::function<void(const std::string&, const std::string&)> cb);
    void set_on_error(std::function<void(int, const std::string&)> cb);

private:
    struct ClientWss {
        std::string id;
        websocket::stream<ssl::stream<tcp::socket>> ws;
        beast::flat_buffer buffer;
        asio::steady_timer ping_timer;

        std::deque<std::shared_ptr<std::string>> write_queue;
        bool writing = false;
        std::atomic<bool> closed{false};

        ClientWss(tcp::socket socket, ssl::context& ctx);
    };

    // Core
    asio::io_context ioc_;
    ssl::context ssl_ctx_;
    tcp::acceptor acceptor_wss_;
    std::thread io_thread_;
    std::atomic<bool> running_{false};

    std::mutex clients_mutex_;
    std::unordered_map<std::string, std::shared_ptr<ClientWss>> clients_wss_;

    // callbacks
    std::function<void(const std::string&)> on_client_connected_;
    std::function<void(const std::string&)> on_client_disconnected_;
    std::function<void(const std::string&, const std::string&)> on_message_;
    std::function<void(int, const std::string&)> on_error_;

    // WSS
    void do_accept_wss();
    void accept_wss(std::shared_ptr<ClientWss> client);
    void do_read(std::shared_ptr<ClientWss> client);
    void do_write(std::shared_ptr<ClientWss> client);
    void start_ping(std::shared_ptr<ClientWss> client);
    void disconnect(std::shared_ptr<ClientWss> client, beast::error_code ec);
};
