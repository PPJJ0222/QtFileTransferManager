#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Qt文件传输管理器");

    MainWindow window;
    window.show();

    return app.exec();
}
