#ifndef LOCALFILEOPS_H
#define LOCALFILEOPS_H

#include <QString>
#include <QStringList>

// 本地文件操作工具类
class LocalFileOps
{
public:
    // 列出目录内容
    static QStringList listDirectory(const QString &path);

    // 复制文件
    static bool copyFile(const QString &source, const QString &dest);

    // 删除文件
    static bool deleteFile(const QString &path);

    // 创建目录
    static bool createDirectory(const QString &path);

    // 重命名
    static bool rename(const QString &oldPath, const QString &newPath);

    // 检查是否为目录
    static bool isDirectory(const QString &path);

    // 获取文件大小
    static qint64 fileSize(const QString &path);
};

#endif // LOCALFILEOPS_H
