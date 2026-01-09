#include <QApplication>
#include <QStyleFactory>
#include "ui/MainWindow.h"
#include "ui/StyleManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Qt文件传输管理器");

#ifdef Q_OS_MAC
    // macOS 使用 Fusion 风格避免原生焦点框问题
    app.setStyle(QStyleFactory::create("Fusion"));
#endif

    // 加载全局样式
    StyleManager::applyStyle(&app);

    MainWindow window;
    window.show();

    return app.exec();
}
