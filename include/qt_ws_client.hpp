#pragma once

#include <QObject>
#include <QString>
#include "ws_api.h"
#include "wss_api.h"

#ifdef _WIN32
#  ifdef WS_DLL_EXPORTS
#    define WS_API __declspec(dllexport)
#  else
#    define WS_API __declspec(dllimport)
#  endif
#else
#  define WS_API
#endif

class QtWsClient : public QObject {
    Q_OBJECT

public:
    explicit QtWsClient(QObject *parent = nullptr);
    ~QtWsClient();

    void connectToServer(const QString &url, const QString &token);
    void disconnectFromServer();
    void sendMessage(const QString &msg);

signals:
    void connected(bool ok);
    void messageReceived(QString msg);
    void errorOccurred(int code, QString text);
    void log(QString text);
    void reconnecting();

private:
    enum class WsMode { WS, WSS };

    ws_handle_t handle_{nullptr};
    WsMode mode_{WsMode::WS};

    static QtWsClient *self_;

    static void on_connected_cb(int connected);
    static void on_message_cb(const char *msg);
    static void on_error_cb(int code, const char *text);

    void destroyHandle();
};


