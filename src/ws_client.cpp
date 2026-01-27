#include "ws_client.hpp"

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


ParsedUrl WsClient::parse_ws_url(const std::string &url, bool is_wss) {
    ParsedUrl result;

    std::string work = url;

    // path
    auto path_pos = work.find('/');
    if (path_pos != std::string::npos) {
        result.path = work.substr(path_pos);
        work = work.substr(0, path_pos);
    }
    else {
        result.path = "/";
    }

    // port
    auto port_pos = work.find(':');
    if (port_pos != std::string::npos) {
        result.host = work.substr(0, port_pos);
        result.port = work.substr(port_pos + 1);
    }
    else {
        result.host = work;
        result.port = is_wss ? "443" : "80";
    }

    return result;
}


void WsClient::do_connect() {
    ParsedUrl parsed = parse_ws_url(last_url_, true);

    // DNS resolve
    resolver_.async_resolve(parsed.host, parsed.port,
        [this, parsed](auto ec, auto results) {
            if (ec) return schedule_reconnect();

            //TCP connect (SYN -> SYN-ACK -> ACK)
            asio::async_connect(ws_.next_layer(), results,
                [this, parsed](auto ec, auto) {
                    if (ec) return schedule_reconnect();

                    // Adding http-headers (bearer, websocket-protocol)
                    ws_.set_option(websocket::stream_base::decorator(
                        [this](auto &req) {
                            req.set("Authorization", "Bearer " + last_token_);
                        }));

                    // WS handshake (HTTP GET, Upgrade)
                    ws_.async_handshake(parsed.host, "/",
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

// Every 5 seconds check if connecton is alive
// reconnect if not
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

// Reading frames, casting to string
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

// Reconnect. Reset timers, wait delay then connect
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

bool WsClient::send(const std::string &msg) {
    if (!connected_) return false;

    try {
        ws_.write(boost::asio::buffer(msg));
        return true;
    }
    catch (const std::exception &e) {
        if (on_error)
            on_error(-3, e.what());
        return false;
    }
}
