#pragma once

#include <QMainWindow>
#include "qt_ws_client.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked();
    void onConnected(bool ok);
    void onLog(QString text);
    void onSendClicked();

private:
    Ui::MainWindow *ui;
    QtWsClient client_;
};

