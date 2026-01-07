#include "LocalFileOps.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

QStringList LocalFileOps::listDirectory(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) return {};

    return dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name);
}

bool LocalFileOps::copyFile(const QString &source, const QString &dest)
{
    // 如果目标已存在，先删除
    if (QFile::exists(dest)) {
        QFile::remove(dest);
    }
    return QFile::copy(source, dest);
}

bool LocalFileOps::deleteFile(const QString &path)
{
    QFileInfo info(path);
    if (info.isDir()) {
        QDir dir(path);
        return dir.removeRecursively();
    }
    return QFile::remove(path);
}

bool LocalFileOps::createDirectory(const QString &path)
{
    QDir dir;
    return dir.mkpath(path);
}

bool LocalFileOps::rename(const QString &oldPath, const QString &newPath)
{
    return QFile::rename(oldPath, newPath);
}

bool LocalFileOps::isDirectory(const QString &path)
{
    QFileInfo info(path);
    return info.isDir();
}

qint64 LocalFileOps::fileSize(const QString &path)
{
    QFileInfo info(path);
    return info.size();
}
