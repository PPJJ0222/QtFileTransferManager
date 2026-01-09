#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <QApplication>
#include <QString>
#include <QIcon>

/**
 * 样式管理器 - 负责加载和应用全局样式
 */
class StyleManager
{
public:
    // 加载并应用全局样式表
    static void applyStyle(QApplication *app);

    // 获取图标
    static QIcon icon(const QString &name);

    // 获取按钮图标（统一黑色）
    static QIcon buttonIcon(const QString &name);

    // 颜色常量
    static constexpr const char* PrimaryColor = "#2D5A8A";
    static constexpr const char* SecondaryColor = "#4A90D9";
    static constexpr const char* HoverColor = "#5BA0E9";
    static constexpr const char* BackgroundColor = "#F5F5F5";
    static constexpr const char* PanelBackground = "#FFFFFF";
    static constexpr const char* BorderColor = "#E0E0E0";
    static constexpr const char* SuccessColor = "#4CAF50";
    static constexpr const char* WarningColor = "#FF9800";
    static constexpr const char* DangerColor = "#E53935";

private:
    StyleManager() = default;
};

#endif // STYLEMANAGER_H
