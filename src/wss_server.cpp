#include "wss_server.hpp"

// ========================
// CLASS FOR WSS CONNECTION
// ========================

// CLIENTS
WssServer::ClientWss::ClientWss(tcp::socket socket, ssl::context& ctx)
    : ws(std::move(socket), ctx),
    ping_timer(ws.get_executor()) {}

//Server
WssServer::WssServer()
    : ssl_ctx_(ssl::context::tls_server),
    acceptor_wss_(ioc_) {

    ssl_ctx_.set_options(
        ssl::context::default_workarounds |
        ssl::context::no_sslv2 |
        ssl::context::no_sslv3 |
        ssl::context::single_dh_use
        );

    ssl_ctx_.use_certificate_chain_file("../certs/server.crt");
    ssl_ctx_.use_private_key_file("../certs/server.key", ssl::context::pem);
}

WssServer::~WssServer() {
    stop();
}

int WssServer::start(int port_wss) {
    try {
        acceptor_wss_.open(tcp::v4());
        acceptor_wss_.set_option(asio::socket_base::reuse_address(true));
        acceptor_wss_.bind({tcp::v4(), static_cast<uint16_t>(port_wss)});
        acceptor_wss_.listen();

        running_ = true;
        do_accept_wss();

        io_thread_ = std::thread([this] { ioc_.run(); });
        return 0;
    } catch (const std::exception& e) {
        if (on_error_) on_error_(-1, e.what());
        return -1;
    }
}

void WssServer::stop() {
    if (!running_) return;
    running_ = false;

    asio::post(ioc_, [this] {
        acceptor_wss_.close();
        clients_wss_.clear();
    });

    ioc_.stop();
    if (io_thread_.joinable())
        io_thread_.join();
}

void WssServer::do_accept_wss() {
    acceptor_wss_.async_accept([this](beast::error_code ec, tcp::socket socket) {
        if (!ec) {
            auto client = std::make_shared<ClientWss>(std::move(socket), ssl_ctx_);
            client->id = std::to_string(reinterpret_cast<uintptr_t>(client.get()));

            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                clients_wss_[client->id] = client;
            }

            accept_wss(client);
        }
        if (running_) do_accept_wss();
    });
}

void WssServer::accept_wss(std::shared_ptr<ClientWss> client) {
    client->ws.next_layer().async_handshake(
        ssl::stream_base::server,
        [this, client](beast::error_code ec) {
            if (ec) return disconnect(client, ec);

            client->ws.set_option(
                websocket::stream_base::timeout::suggested(beast::role_type::server));

            client->ws.async_accept([this, client](beast::error_code ec2) {
                if (ec2) return disconnect(client, ec2);

                if (on_client_connected_) on_client_connected_(client->id);
                start_ping(client);
                do_read(client);
            });
        });
}

void WssServer::do_read(std::shared_ptr<ClientWss> client) {
    client->ws.async_read(client->buffer,
                          [this, client](beast::error_code ec, std::size_t bytes) {
                              if (ec) return disconnect(client, ec);

                              std::string msg = beast::buffers_to_string(client->buffer.data());
                              client->buffer.consume(bytes);

                              if (on_message_) on_message_(client->id, msg);

                              client->write_queue.push_back(
                                  std::make_shared<std::string>(msg));

                              if (!client->writing)
                                  do_write(client);

                              do_read(client);
                          });
}

void WssServer::do_write(std::shared_ptr<ClientWss> client) {
    if (client->write_queue.empty()) {
        client->writing = false;
        return;
    }

    client->writing = true;
    auto msg = client->write_queue.front();

    client->ws.text(true);
    client->ws.async_write(asio::buffer(*msg),
                           [this, client](beast::error_code ec, std::size_t) {
                               if (ec) return disconnect(client, ec);

                               client->write_queue.pop_front();
                               do_write(client);
                           });
}

void WssServer::start_ping(std::shared_ptr<ClientWss> client) {
    client->ping_timer.expires_after(std::chrono::seconds(5));
    client->ping_timer.async_wait([this, client](beast::error_code ec) {
        if (ec) return;
        client->ws.async_ping({}, [this, client](beast::error_code ec2) {
            if (ec2) disconnect(client, ec2);
        });
        start_ping(client);
    });
}

void WssServer::disconnect(std::shared_ptr<ClientWss> client, beast::error_code) {
    if (client->closed.exchange(true)) return;

    if (on_client_disconnected_) on_client_disconnected_(client->id);
}

// Callback setters
void WssServer::set_on_client_connected(std::function<void(const std::string&)> cb) {
    on_client_connected_ = std::move(cb);
}
void WssServer::set_on_client_disconnected(std::function<void(const std::string&)> cb) {
    on_client_disconnected_ = std::move(cb);
}
void WssServer::set_on_message(std::function<void(const std::string&, const std::string&)> cb) {
    on_message_ = std::move(cb);
}
void WssServer::set_on_error(std::function<void(int, const std::string&)> cb) {
    on_error_ = std::move(cb);
}
