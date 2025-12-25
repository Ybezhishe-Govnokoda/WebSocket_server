#include "main.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent),
   ui(new Ui::MainWindow),
   client_(new WsClientQt(this))
{
   ui->setupUi(this);

   connect(ui->btnConnect, &QPushButton::clicked,
      this, &MainWindow::onConnectClicked);
   connect(ui->btnDisconnect, &QPushButton::clicked,
      this, &MainWindow::onDisconnectClicked);
   connect(ui->btnSend, &QPushButton::clicked,
      this, &MainWindow::onSendClicked);

   connect(client_, &WsClientQt::connected,
      this, &MainWindow::onConnected);
   connect(client_, &WsClientQt::messageReceived,
      this, &MainWindow::onMessage);
   connect(client_, &WsClientQt::errorOccurred,
      this, &MainWindow::onError);
}

MainWindow::~MainWindow() {
   delete ui;
}

void MainWindow::onConnectClicked() {
   client_->connectToServer(
      ui->editUrl->text(),
      ui->editToken->text()
   );
}

void MainWindow::onDisconnectClicked() {
   client_->disconnectFromServer();
}

void MainWindow::onSendClicked() {
   client_->sendMessage(ui->editMessage->text());
   ui->editMessage->clear();
}

void MainWindow::onConnected(bool ok) {
   ui->log->append(ok ? "[CONNECTED]" : "[DISCONNECTED]");
}

void MainWindow::onMessage(QString msg) {
   ui->log->append("[MESSAGE] " + msg);
}

void MainWindow::onError(int code, QString text) {
   ui->log->append(
      QString("[ERROR %1] %2").arg(code).arg(text)
   );
}
