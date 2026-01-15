#include "wss_client.hpp"
#include <iostream>

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

    if (url.rfind("wss://", 0) == 0) {
        mode_ = WsMode::WSS;
        last_url_ = url.substr(6);
    } else if (url.rfind("ws://", 0) == 0) {
        std::cout << "WS connection unavailable";
        return 0;
        //mode_ = WsMode::WS;
        //last_url_ = url.substr(5);
    } else {
        if (on_error)
            on_error(-1, "Invalid URL scheme (use ws:// or wss://)");
        return -1;
    }

    do_connect();
    io_thread_ = std::thread([this] { ioc_.run(); });
    return 0;
}

void WssClient::do_connect() {
    asio::dispatch(strand_, [this] {
        auto pos = last_url_.find(':');
        auto host = last_url_.substr(0, pos);
        auto port = last_url_.substr(pos + 1);

        resolver_.async_resolve(
            host, port,
            asio::bind_executor(strand_,
                [this, host](auto ec, auto results) {
                    if (ec) return schedule_reconnect();

                    if (mode_ == WsMode::WS) {
                        std::cout << "WS connection unavailable";
                        return;
                        /*
                        ws_ = std::make_unique<websocket::stream<tcp::socket>>(strand_);

                        asio::async_connect(
                            ws_->next_layer(), results,
                            asio::bind_executor(strand_,
                                [this, host](auto ec, auto) {
                                    if (ec) return schedule_reconnect();

                                        ws_->async_handshake(
                                            host, "/",
                                                asio::bind_executor(strand_,
                                                    [this](auto ec) {
                                                        if (ec) return schedule_reconnect();
                                                        connected_ = true;
                                                        reconnect_delay_ = 1;
                                                        if (on_connected) on_connected(true);
                                                        start_ping();
                                                        do_read();
                                                    }));
                                }));
                            */
                    } else {
                        ssl_ctx_.set_verify_mode(ssl::verify_none);
                        wss_ = std::make_unique<
                            websocket::stream<ssl::stream<tcp::socket>>
                            >(strand_, ssl_ctx_);

                        asio::async_connect(
                            wss_->next_layer().next_layer(), results,
                            asio::bind_executor(strand_,
                                [this, host](auto ec, auto) {
                                    if (ec) return schedule_reconnect();

                                    wss_->next_layer().async_handshake(
                                        ssl::stream_base::client,
                                        asio::bind_executor(strand_,
                                            [this, host](auto ec) {
                                                if (ec) return schedule_reconnect();

                                                wss_->async_handshake(
                                                    host, "/",
                                                    asio::bind_executor(strand_,
                                                        [this](auto ec) {
                                                            if (ec) return schedule_reconnect();
                                                            connected_ = true;
                                                            reconnect_delay_ = 1;
                                                            if (on_connected) on_connected(true);
                                                            start_ping();
                                                            do_read();
                                                        }));
                                                }));
                                    }));
                        }
                }));
    });
}


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

                if (mode_ == WsMode::WS) {
                    std::cout << "WS connection unavailable";
                    return;
                    //ws_->async_ping({}, handler);
                }
                else
                    wss_->async_ping({}, handler);

                start_ping();
                }));
}

void WssClient::do_read() {
    auto buffer = std::make_shared<beast::flat_buffer>();

    if (mode_ == WsMode::WS) {
        std::cout << "WS connection unavailable";
        return;
        /*
        ws_->async_read(
            *buffer,
            asio::bind_executor(strand_,
                [this, buffer](auto ec, auto) {
                    if (ec) return schedule_reconnect();
                    if (on_message)
                        on_message(beast::buffers_to_string(buffer->data()));
                    do_read();
                }));
        */
    } else {
        wss_->async_read(
            *buffer,
            asio::bind_executor(strand_,
                [this, buffer](auto ec, auto) {
                    if (ec) return schedule_reconnect();
                    if (on_message)
                        on_message(beast::buffers_to_string(buffer->data()));
                    do_read();
                }));
    }
}

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
                                    if (ec) return;
                                    reconnect_delay_ = std::min(reconnect_delay_ * 2, 10);
                                    do_connect();
                                }));
    });
}

void WssClient::disconnect() {
    asio::post(ioc_, [this] {
        beast::error_code ec;
        if (mode_ == WsMode::WS /* && ws_ */) {
            std::cout << "WS connection unavailable";
            return;
            //ws_->close(websocket::close_code::normal, ec);
        }
        if (mode_ == WsMode::WSS && wss_)
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

        if (mode_ == WsMode::WS) {
            std::cout << "WS connection unavailable";
            return;
            /*
            ws_->async_write(
                asio::buffer(msg),
                asio::bind_executor(strand_,
                    [this](beast::error_code ec, std::size_t) {
                        if (ec) schedule_reconnect();
                    }));
            */
        } else {
            wss_->async_write(
                asio::buffer(msg),
                asio::bind_executor(strand_,
                    [this](beast::error_code ec, std::size_t) {
                        if (ec) schedule_reconnect();
                    }));
        }
    });

    return true;
}
