#ifndef FILEINFO_H
#define FILEINFO_H

#include <QString>
#include <QDateTime>

// 统一的文件信息结构体
struct FileInfo
{
    QString name;
    QString path;
    qint64 size = 0;
    QDateTime modifiedTime;
    bool isDirectory = false;

    FileInfo() = default;
    FileInfo(const QString &n, const QString &p, qint64 s, bool isDir)
        : name(n), path(p), size(s), isDirectory(isDir) {}
};

#endif // FILEINFO_H
