#pragma once
#include <QObject>
#include "ws_api.h"

class WsClientQt : public QObject {
   Q_OBJECT

public:
   explicit WsClientQt(QObject *parent = nullptr);
   ~WsClientQt();

   void connectToServer(const QString &url, const QString &token);
   void disconnectFromServer();
   void sendMessage(const QString &msg);

signals:
   void connected(bool ok);
   void messageReceived(QString msg);
   void errorOccurred(int code, QString text);

private:
   ws_handle_t handle_;

   static void on_connected_cb(int c);
   static void on_message_cb(const char *msg);
   static void on_error_cb(int code, const char *text);

   static WsClientQt *instance_; // для bridge
};
