
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMenu>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QtWebSockets>
#include <QWebSocket>
#include <QWebSocketServer>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget {
Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);

    ~Widget();

protected:
    /// 重写鼠标右键菜单事件
    void contextMenuEvent(QContextMenuEvent *event) override;
    /// 重写窗口关闭事件
    void closeEvent(QCloseEvent *event) override;


private:
    Ui::Widget *ui;

    /// 托盘
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(this);

    /// 鼠标右键菜单
    QMenu *mouseMenu = new QMenu;
    QAction *open;
    QAction *add;
    QAction *del;
    void menuConfig();

    /// 托盘右键菜单
    QMenu *trayMenu = new QMenu;
    QAction *exitApp;

    /// 存储列表(暂时没用)
    struct tbaleList{
        QString gesture;
        QString path;
    };
    QList<tbaleList> list;
    QSettings* info;

    /// WebSocket服务器
    QWebSocketServer *mWebSocketServer;
    QWebSocket *webSocket;
    void runServer();

    /// 打开软件
    void openApp(QString order);

    /// 转换接收结果
    void convert(QString message);

};

#endif // WIDGET_H
