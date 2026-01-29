#include <QApplication>
#include <QCommandLineParser>
#include "web/WebWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Qt文件传输管理器");

    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt文件传输管理器");
    parser.addHelpOption();

    // --dev 开发模式（加载 localhost:5173）
    QCommandLineOption devOption("dev", "开发模式");
    parser.addOption(devOption);

    parser.process(app);

    bool devMode = parser.isSet(devOption);

    // WebEngine 模式
    WebWindow window(devMode);
    window.show();
    return app.exec();
}
