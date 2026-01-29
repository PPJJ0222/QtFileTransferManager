#ifndef WEBWINDOW_H
#define WEBWINDOW_H

#include <QMainWindow>

class QWebEngineView;
class QWebChannel;
class Bridge;

// 基于 WebEngine 的主窗口
class WebWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit WebWindow(bool devMode = false, QWidget *parent = nullptr);
    ~WebWindow();

private:
    void setupWebView();
    void setupWebChannel();

    QWebEngineView *m_webView;
    QWebChannel *m_channel;
    Bridge *m_bridge;
    bool m_devMode;
};

#endif // WEBWINDOW_H
