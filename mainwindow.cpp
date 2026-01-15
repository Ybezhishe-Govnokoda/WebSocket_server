#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->connectBtn, &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);

    connect(&client_, &QtWsClient::connected,
            this, &MainWindow::onConnected);

    connect(&client_, &QtWsClient::log,
            this, &MainWindow::onLog);

    connect(ui->sendBtn, &QPushButton::clicked,
            this, &MainWindow::onSendClicked);

    connect(&client_, &QtWsClient::messageReceived,
            this, &MainWindow::onMessageReceived);

    // Connection inputs
    ui->hostEdit->setVisible(!connected_);
    ui->tokenEdit->setVisible(!connected_);

    // Messaging UI
    ui->messageEdit->setVisible(connected_);
    ui->sendBtn->setVisible(connected_);

    ui->connectBtn->setText(connected_ ? "Disconnect" : "Connect");
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onConnectClicked() {
    if (!connected_) {
        client_.connectToServer(
            ui->hostEdit->text(),
            ui->tokenEdit->text()
            );
    } else {
        //ui->connectBtn->setText(connected_ ? "Disconnect" : "Connect");

        client_.disconnectFromServer();
    }
}

void MainWindow::onConnected(bool ok) {
    connected_ = ok;

    // Connection inputs
    ui->hostEdit->setVisible(!ok);
    ui->tokenEdit->setVisible(!ok);

    // Messaging UI
    ui->messageEdit->setVisible(ok);
    ui->sendBtn->setVisible(ok);

    ui->connectBtn->setText(ok ? "Disconnect" : "Connect");
}

void MainWindow::onLog(QString text) {
    ui->logView->append(text);
}

void MainWindow::onSendClicked() {
    QString text = ui->messageEdit->text();
    if (text.isEmpty())
        return;

    client_.sendMessage(text);
    ui->messageEdit->clear();
}

void MainWindow::onMessageReceived(QString text) {
    ui->logView->append("[SERVER] " + text);
}

