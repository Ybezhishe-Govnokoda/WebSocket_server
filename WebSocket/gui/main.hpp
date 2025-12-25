#pragma once
#include <QMainWindow>
#include "ws_client.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
   Q_OBJECT

public:
   MainWindow(QWidget *parent = nullptr);
   ~MainWindow();

private slots:
   void onConnectClicked();
   void onDisconnectClicked();
   void onSendClicked();

   void onConnected(bool ok);
   void onMessage(QString msg);
   void onError(int code, QString text);

private:
   Ui::MainWindow *ui;
   WsClientQt *client_;
};
