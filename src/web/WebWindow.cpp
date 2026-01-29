#include "WebWindow.h"
#include "Bridge.h"
#include <QWebEngineView>
#include <QWebChannel>
#include <QUrl>

WebWindow::WebWindow(bool devMode, QWidget *parent)
    : QMainWindow(parent)
    , m_webView(new QWebEngineView(this))
    , m_channel(new QWebChannel(this))
    , m_bridge(new Bridge(this))
    , m_devMode(devMode)
{
    setWindowTitle("文件传输管理器");
    resize(1200, 700);

    setupWebChannel();
    setupWebView();

    setCentralWidget(m_webView);
}

WebWindow::~WebWindow() = default;

void WebWindow::setupWebChannel()
{
    m_channel->registerObject("bridge", m_bridge);
    m_webView->page()->setWebChannel(m_channel);
}

void WebWindow::setupWebView()
{
    if (m_devMode) {
        // 开发模式：加载 Vue 开发服务器
        m_webView->setUrl(QUrl("http://localhost:5173"));
    } else {
        // 发布模式：加载嵌入资源
        m_webView->setUrl(QUrl("qrc:/web/index.html"));
    }
}
