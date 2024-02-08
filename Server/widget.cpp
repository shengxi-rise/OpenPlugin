#include "widget.h"
#include "./ui_widget.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QSettings>
#include <QProcess>
#include <QMessageBox>

///*
/// 当前程序思路：
/// 启动程序先导入配置文件，写入表格
/// 用户可以直接在表格上进行编辑
/// 不允许关闭窗口，只能在托盘中关闭
/// 关闭的时候将表格数据写入配置文件
///
/// *///

/// BUG: 服务端：保存的表格内容顺序混乱，频繁打开软件出错，// 最小化时出错（原因找到了，把提示框关掉就行） 程序不是bug关闭，而是正常关闭。。。

Widget::Widget(QWidget *parent)
        : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    ui->AppTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);   // 设置表头自动填充

    /// WebSocket服务器
    runServer();

    /// 配置文件
    info = new QSettings("../config/config.ini", QSettings::IniFormat);

    /// 读取配置文件写入表格
    QStringList keys = info->allKeys();
    int i = 0;
            foreach(QString key, keys) {
            QString value = info->value(key).toString();
            /// 渲染表格，
            /// （这里犯了一个错，直接把item->setText(),这样子是不行的，因为本来不存在item）
            // 创建 QTableWidgetItem 对象
            QTableWidgetItem *keyItem = new QTableWidgetItem;
            QTableWidgetItem *valueItem = new QTableWidgetItem;

            // 设置单元格内容
            keyItem->setText(key);
            valueItem->setText(value);

            // 将单元格添加到表格中
            ui->AppTable->setItem(i, 1, keyItem);
            ui->AppTable->setItem(i, 2, valueItem);
            i++;
        }


    /// 托盘对象
    trayIcon->setIcon(QIcon("../resources/cat.ico"));
    trayIcon->setToolTip("openplugin");

    // 托盘菜单
    exitApp = new QAction("exit", this);
    trayMenu->addAction(exitApp);
    trayIcon->setContextMenu(trayMenu);
    connect(exitApp, &QAction::triggered, this, [&] {
        // 保存表格中的内容
        info->clear();
        for (int i = 0; i < ui->AppTable->rowCount(); ++i) {
            if (ui->AppTable->item(i, 1)) {
                info->setValue(ui->AppTable->item(i, 1)->text(), ui->AppTable->item(i, 2)->text());
            }
        }

        info->sync();
        // 退出程序
        exit(0);
    });
    // 点击托盘图标
    connect(trayIcon, &QSystemTrayIcon::activated, this, [&] {
        qDebug() << "clicked";
        this->show();
    });


    /// 鼠标右键菜单
    // 创建事件
    add = new QAction(("Add"), this);
    del = new QAction(("Del"), this);
    open = new QAction(("Open"), this);
    // 添加进菜单
    mouseMenu->addAction(add);
    mouseMenu->addAction(del);
    mouseMenu->addAction(open);


    /// 打开软件测试菜单
    connect(open, &QAction::triggered, this, [&] {
        // 打开软件
        QProcess process;
        QString path = ui->AppTable->item(0, 2)->text();
        process.start(path);
        process.waitForFinished();

    });

}

/// 重写鼠标右键事件
void Widget::contextMenuEvent(QContextMenuEvent *event) {
    // 打开菜单
    mouseMenu->exec(event->globalPos());
}

/// 重写关闭窗口事件
void Widget::closeEvent(QCloseEvent *event) {
    QMessageBox::information(this, "WARNNING", "最小化到托盘");
    this->hide();       // 隐藏窗口
    trayIcon->show();   // 显示托盘
    event->ignore();    // 阻止关闭事件
}

/// 启动WebSocket服务器
void Widget::runServer() {
    // 创建 WebSocket 服务器
    mWebSocketServer = new QWebSocketServer(u8"server", QWebSocketServer::NonSecureMode);
    // 监听端口
    mWebSocketServer->listen(QHostAddress::Any, 8765);
    // 连接信号和槽
    connect(mWebSocketServer, &QWebSocketServer::newConnection, this, [&] {
        // 接受连接
        webSocket = mWebSocketServer->nextPendingConnection();
        QMessageBox::information(this, "Tips", "客户端连接成功");
        // 接收信息
        connect(webSocket, &QWebSocket::textMessageReceived, this, [=](const QString &message) {
            qDebug() << "Received message: " << message;

            // 模型结果转换（用的官方模型），需要做个转换
            convert(message);

            // 打开软件
            // openApp(message);
        });
        // 连接成功提示
        webSocket->sendTextMessage("Hello from Qt!");
        // 断开状态提示
        connect(webSocket, &QWebSocket::disconnected, this, [&] {
            QMessageBox::information(this, "WARNNING", "客户端断开连接");
        });
    });
}


/// 结果转换
void Widget::convert(QString message) {
    QString order;
    if (message == "Victory") {
        order = "0";
    } else if (message == "Pointing_Up") {
        order = "1";
    } else if (message == "Thumb_Up") {
        order = "2";
    } else if (message == "Open_Palm") {
        order = "3";
    } else if (message == "Thumb_Down") {
        order = "4";
    } else {
        order = "-1";
    }
    openApp(order);
}


/// TODO: 用户可自定义的程度取决于：插件设置的动作以及实际执行的逻辑，模型推理和返回的数据
/// 不变的是模型的输出，能变的是打开的软件（路径）
void Widget::openApp(QString order) {
    QProcess process;
    if (order == "-1") {
        return;
    }
    qDebug() << "order:" << order.toInt();
    if (ui->AppTable->item(order.toInt(), 2)) {
        qDebug() << order.toInt();
        QString path = ui->AppTable->item(order.toInt(), 2)->text();
        process.start(path);
        process.waitForFinished();
    } else {
        // 总是提示太烦了
//        QMessageBox::information(this, "WARNNING", "未设置应用程序");
    }

}


void Widget::menuConfig() {

}


Widget::~Widget() {
    delete ui;
}






