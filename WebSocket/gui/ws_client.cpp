#include "ws_client.hpp"

WsClientQt *WsClientQt::instance_ = nullptr;

WsClientQt::WsClientQt(QObject *parent)
   : QObject(parent)
{
   instance_ = this;
   handle_ = ws_client_create();

   ws_client_set_on_connected(handle_, &WsClientQt::on_connected_cb);
   ws_client_set_on_message(handle_, &WsClientQt::on_message_cb);
   ws_client_set_on_error(handle_, &WsClientQt::on_error_cb);
}

WsClientQt::~WsClientQt() {
   ws_client_disconnect(handle_);
   ws_client_destroy(handle_);
   instance_ = nullptr;
}

void WsClientQt::connectToServer(const QString &url, const QString &token) {
   ws_client_connect(handle_, url.toUtf8().constData(),
      token.toUtf8().constData());
}

void WsClientQt::disconnectFromServer() {
   ws_client_disconnect(handle_);
}

void WsClientQt::sendMessage(const QString &msg) {
   ws_client_send(handle_, msg.toUtf8().constData());
}

/* ===== static callbacks ===== */

void WsClientQt::on_connected_cb(int c) {
   if (instance_)
      emit instance_->connected(c != 0);
}

void WsClientQt::on_message_cb(const char *msg) {
   if (instance_)
      emit instance_->messageReceived(QString::fromUtf8(msg));
}

void WsClientQt::on_error_cb(int code, const char *text) {
   if (instance_)
      emit instance_->errorOccurred(code, QString::fromUtf8(text));
}
