#include "StyleManager.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>

void StyleManager::applyStyle(QApplication *app)
{
    QFile styleFile(":/styles/main.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = styleFile.readAll();
        app->setStyleSheet(style);
        styleFile.close();
        qDebug() << "样式表加载成功";
    } else {
        qWarning() << "无法加载样式表: :/styles/main.qss";
    }
}

QIcon StyleManager::icon(const QString &name)
{
    return QIcon(QString(":/icons/%1.svg").arg(name));
}

QIcon StyleManager::buttonIcon(const QString &name)
{
    QFile file(QString(":/icons/%1.svg").arg(name));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return QIcon();
    }
    QString svgContent = file.readAll();
    file.close();

    // 将 stroke 颜色替换为黑色
    svgContent.replace(QRegularExpression("stroke=\"#[0-9A-Fa-f]{6}\""), "stroke=\"#000000\"");

    QPixmap pixmap;
    pixmap.loadFromData(svgContent.toUtf8(), "SVG");
    return QIcon(pixmap);
}
