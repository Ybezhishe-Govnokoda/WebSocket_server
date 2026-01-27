#include "wss_client.hpp"

WssClient::WssClient()
    : strand_(asio::make_strand(ioc_)),
    resolver_(ioc_),
    ssl_ctx_(ssl::context::tls_client),
    ping_timer_(ioc_),
    pong_timeout_(ioc_),
    reconnect_timer_(ioc_) {
}

WssClient::~WssClient() {
    disconnect();
}

int WssClient::connect(const std::string &url, const std::string &token) {
    last_url_ = url;
    last_token_ = token;

    do_connect();
    io_thread_ = std::thread([this] { ioc_.run(); });
    return 0;
}

ParsedUrl WssClient::parse_ws_url(const std::string &url, bool is_wss) {
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

void WssClient::do_connect() {
    asio::dispatch(strand_, [this] {
        ParsedUrl parsed = parse_ws_url(last_url_, true);

        // DNS resolve
        resolver_.async_resolve(
            parsed.host,
            parsed.port,
            asio::bind_executor(strand_,
                [this, parsed](auto ec, auto results) {
                    if (ec) {
                        if (on_error)
                            on_error(ec.value(), "Resolve failed: " + ec.message());
                        return schedule_reconnect();
                    }
                    if (on_error)
                        on_error(0, "Resolve OK");

                    // TLS init
                    ssl_ctx_.set_default_verify_paths();
                    ssl_ctx_.set_verify_mode(ssl::verify_peer);
                    ssl_ctx_.load_verify_file("cacert.pem");

                    wss_ = std::make_unique<
                        websocket::stream<ssl::stream<tcp::socket>>
                        >(strand_, ssl_ctx_);

                    if (on_error) on_error(0, "TCP connecting...");
                    asio::async_connect(
                        //TCP connect (SYN -> SYN-ACK -> ACK)
                        wss_->next_layer().next_layer(), results,
                        asio::bind_executor(strand_,
                            [this, parsed](auto ec, auto) {

                            if (ec) {
                                if (on_error)
                                    on_error(ec.value(),
                                    "TCP connect failed: " + ec.message());
                                return schedule_reconnect();
                            }
                            if (on_error)
                                on_error(0, "TCP connected");

                            // Server name indication, checking cert
                            if (!SSL_set_tlsext_host_name(
                                    wss_->next_layer().native_handle(),
                                    parsed.host.c_str()))
                            {
                                beast::error_code ec{
                                    static_cast<int>(::ERR_get_error()),
                                    asio::error::get_ssl_category()
                                };
                                if (on_error) on_error(ec.value(), ec.message());
                                return schedule_reconnect();
                            }

                            // Adding http-headers (bearer, websocket-protocol)
                            wss_->set_option(websocket::stream_base::decorator(
                                [this](websocket::request_type& req) {
                                    req.set(beast::http::field::authorization,
                                            "Bearer " + last_token_);
                                    req.set(beast::http::field::sec_websocket_protocol, "bearer");
                                }));

                            // TLS handshake
                            if (on_error) on_error(0, "Setting SNI + SSL handshake");
                            wss_->next_layer().async_handshake(
                                ssl::stream_base::client,
                                asio::bind_executor(strand_,
                                    [this, parsed](auto ec) {
                                        if (ec) {
                                            if (on_error)
                                                on_error(ec.value(),
                                                "SSL handshake failed: " +
                                                ec.message());
                                            return schedule_reconnect();
                                        }
                                        if (on_error)
                                            on_error(0, "SSL handshake OK");

                                        auto res = std::make_shared<websocket::response_type>();
                                        if (on_error)
                                            on_error(0, "WS handshake starting");


                                        // WS handshake (HTTP GET, Upgrade)
                                        wss_->async_handshake(
                                            *res,
                                            parsed.host,
                                            parsed.path,
                                            asio::bind_executor(strand_,
                                                [this, res](auto ec) {
                                                    if (ec) {
                                                        if (on_error) {
                                                            on_error(ec.value(),
                                                            "WS handshake failed: " + ec.message() +
                                                            "\nHTTP status: " + std::to_string(res->result_int()));
                                                        }
                                                        return schedule_reconnect();
                                                    }
                                                    if (on_error)
                                                        on_error(0, "WS handshake OK");

                                                    // Successful connection
                                                    connected_ = true;
                                                    reconnect_delay_ = 1;
                                                    if (on_connected) on_connected(true);

                                                    start_ping();
                                                    do_read();
                                                }));
                                    }));
                            }));

                }));
    });
}


// Every 5 seconds check if connecton is alive
// reconnect if not
void WssClient::start_ping() {
    ping_timer_.expires_after(std::chrono::seconds(5));
    ping_timer_.async_wait(
        asio::bind_executor(strand_,
            [this](auto ec) {
                if (ec) return;

                auto handler = asio::bind_executor(
                    strand_,
                    [this](auto ec) {
                        if (ec) schedule_reconnect();
                    });

                wss_->async_ping({}, handler);

                start_ping();
                }));
}

// Reading frames, casting to string
void WssClient::do_read() {
    auto buffer = std::make_shared<beast::flat_buffer>();

    wss_->async_read(
        *buffer,
        asio::bind_executor(strand_,
            [this, buffer](auto ec, auto) {
                if (ec) {
                    if (on_error) on_error(ec.value(), ec.message());
                    return schedule_reconnect();
                }

                if (on_message)
                    on_message(beast::buffers_to_string(buffer->data()));
                do_read();
            }));
}

// Reconnect. Reset timers, wait delay then connect
void WssClient::schedule_reconnect() {
    asio::dispatch(strand_, [this] {
        if (connected_) {
            connected_ = false;
            if (on_connected) on_connected(false);
        }

        ping_timer_.cancel();
        pong_timeout_.cancel();

        reconnect_timer_.expires_after(
            std::chrono::seconds(reconnect_delay_));

        reconnect_timer_.async_wait(
            asio::bind_executor(strand_,
                [this](auto ec) {
                    if (ec) {
                        if (on_error) on_error(ec.value(), ec.message());
                        return;
                    }
                    reconnect_delay_ = std::min(reconnect_delay_ * 2, 10);
                    do_connect();
                }));
    });
}

// Closes the socket, stop io_contaxt join thread
void WssClient::disconnect() {
    asio::post(ioc_, [this] {
        beast::error_code ec;
        if (wss_)
            wss_->close(websocket::close_code::normal, ec);
    });

    ioc_.stop();
    if (io_thread_.joinable())
        io_thread_.join();
}


bool WssClient::send(const std::string &msg) {
    if (!connected_) return false;

    asio::post(strand_, [this, msg] {
        if (!connected_) return;

        wss_->async_write(
            asio::buffer(msg),
            asio::bind_executor(strand_,
                [this](beast::error_code ec, std::size_t) {
                    if (ec) schedule_reconnect();
                }));
    });

    return true;
}
