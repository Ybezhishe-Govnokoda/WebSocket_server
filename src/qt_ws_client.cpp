#include "qt_ws_client.hpp"

QtWsClient *QtWsClient::self_ = nullptr;

QtWsClient::QtWsClient(QObject *parent)
    : QObject(parent)
{
    self_ = this;
}

QtWsClient::~QtWsClient() {
    destroyHandle();
}


void QtWsClient::connectToServer(const QString &url, const QString &token) {
    destroyHandle();

    QString cleanUrl;

    if (url.startsWith("wss://")) {
        mode_ = WsMode::WSS;
        cleanUrl = url.mid(6);
        emit log("Using WSS, loading cacert.pem");
    }
    else if (url.startsWith("ws://")) {
        mode_ = WsMode::WS;
        cleanUrl = url.mid(5);
        emit log("Using WS, loading cacert.pem");
    }
    else {
        emit errorOccurred(-1, "Invalid URL scheme");
        emit log("Invalid URL scheme");
        return;
    }

    if (mode_ == WsMode::WS) {
        handle_ = ws_client_create();

        ws_client_set_on_connected(handle_, on_connected_cb);
        ws_client_set_on_message(handle_, on_message_cb);
        ws_client_set_on_error(handle_, on_error_cb);

        ws_client_connect(
            handle_,
            cleanUrl.toUtf8().constData(),
            token.toUtf8().constData()
            );
    }
    else {
        handle_ = wss_client_create();

        wss_client_set_on_connected(handle_, on_connected_cb);
        wss_client_set_on_message(handle_, on_message_cb);
        wss_client_set_on_error(handle_, on_error_cb);

        wss_client_connect(
            handle_,
            cleanUrl.toUtf8().constData(),
            token.toUtf8().constData()
            );
    }

    emit log("Connecting to " + cleanUrl);
}


void QtWsClient::disconnectFromServer() {
    emit log("Disconnect requested");
    destroyHandle();
    emit connected(false);
}

void QtWsClient::sendMessage(const QString &msg) {
    if (!handle_) return;

    if (mode_ == WsMode::WS) {
        ws_client_send(handle_, msg.toUtf8().constData());
    } else {
        wss_client_send(handle_, msg.toUtf8().constData());
    }
}

// C callbacks
void QtWsClient::on_connected_cb(int connected) {
    if (!self_) return;

    emit self_->connected(connected);
    emit self_->log(connected ? "Connected" : "Disconnected");

    if (!connected)
        emit self_->reconnecting();
}

void QtWsClient::on_message_cb(const char *msg) {
    if (!self_) return;
    emit self_->messageReceived(QString::fromUtf8(msg));
}

void QtWsClient::on_error_cb(int code, const char *text) {
    if (!self_) return;
    emit self_->errorOccurred(code, QString::fromUtf8(text));
}

void QtWsClient::destroyHandle() {
    if (!handle_) return;

    if (mode_ == WsMode::WS) {
        ws_client_disconnect(handle_);
        ws_client_destroy(handle_);
    } else {
        wss_client_disconnect(handle_);
        wss_client_destroy(handle_);
    }

    handle_ = nullptr;
}

