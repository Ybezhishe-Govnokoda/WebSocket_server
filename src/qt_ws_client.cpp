#include "qt_ws_client.h"

QtWsClient *QtWsClient::self_ = nullptr;

QtWsClient::QtWsClient(QObject *parent)
    : QObject(parent)
{
    self_ = this;
    handle_ = ws_client_create();

    ws_client_set_on_connected(handle_, on_connected_cb);
    ws_client_set_on_message(handle_, on_message_cb);
    ws_client_set_on_error(handle_, on_error_cb);
}

QtWsClient::~QtWsClient() {
    ws_client_disconnect(handle_);
    ws_client_destroy(handle_);
}

void QtWsClient::connectToServer(const QString &host, const QString &token) {
    emit log("Connecting to " + host);
    ws_client_connect(handle_,
                      host.toUtf8().constData(),
                      token.toUtf8().constData());
}

void QtWsClient::disconnectFromServer() {
    emit log("Disconnect requested");
    ws_client_disconnect(handle_);
}

void QtWsClient::sendMessage(const QString &msg) {
    ws_client_send(handle_, msg.toUtf8().constData());
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

    QString text = QString::fromUtf8(msg);

    if (text == "pong") {
        emit self_->pongReceived();
        emit self_->log("PONG received");
    } else {
        emit self_->messageReceived(text);
        emit self_->log("Message: " + text);
    }
}

void QtWsClient::on_error_cb(int code, const char *text) {
    if (!self_) return;

    emit self_->errorOccurred(code, QString::fromUtf8(text));
    emit self_->log(QString("Error %1: %2").arg(code).arg(text));
}
