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
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onConnectClicked() {
    client_.connectToServer(
        ui->hostEdit->text(),
        ui->tokenEdit->text()
        );
}

void MainWindow::onConnected(bool ok) {
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


