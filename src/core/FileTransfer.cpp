#include "FileTransfer.h"
#include "FtpClient.h"
#include <QFileInfo>

FileTransfer::FileTransfer(QObject *parent)
    : QObject(parent)
    , m_ftpClient(nullptr)
    , m_machineFtpClient(nullptr)
{
}

bool FileTransfer::ftpToLocal(const QString &remotePath, const QString &localPath)
{
    if (!m_ftpClient) {
        emit transferComplete(false, "FTP客户端未设置");
        return false;
    }

    emit progressChanged(0);
    bool success = m_ftpClient->downloadFile(remotePath, localPath);
    emit progressChanged(100);
    emit transferComplete(success, success ? "下载完成" : "下载失败");
    return success;
}

bool FileTransfer::localToFtp(const QString &localPath, const QString &remotePath)
{
    if (!m_ftpClient) {
        emit transferComplete(false, "FTP客户端未设置");
        return false;
    }

    emit progressChanged(0);
    bool success = m_ftpClient->uploadFile(localPath, remotePath);
    emit progressChanged(100);
    emit transferComplete(success, success ? "上传完成" : "上传失败");
    return success;
}

bool FileTransfer::machineToLocal(const QString &remotePath, const QString &localPath)
{
    if (!m_machineFtpClient) {
        emit transferComplete(false, "机床FTP客户端未设置");
        return false;
    }

    emit progressChanged(0);
    bool success = m_machineFtpClient->downloadFile(remotePath, localPath);
    emit progressChanged(100);
    emit transferComplete(success, success ? "下载完成" : "下载失败");
    return success;
}

bool FileTransfer::localToMachine(const QString &localPath, const QString &remotePath)
{
    if (!m_machineFtpClient) {
        emit transferComplete(false, "机床FTP客户端未设置");
        return false;
    }

    emit progressChanged(0);
    bool success = m_machineFtpClient->uploadFile(localPath, remotePath);
    emit progressChanged(100);
    emit transferComplete(success, success ? "上传完成" : "上传失败");
    return success;
}

void FileTransfer::cancelTransfer()
{
    m_cancelRequested = true;
}

bool FileTransfer::ftpToLocalBatch(const QStringList &remotePaths, const QStringList &localPaths)
{
    if (!m_ftpClient || remotePaths.size() != localPaths.size()) {
        emit transferComplete(false, "参数错误");
        return false;
    }

    m_isTransferring = true;
    m_cancelRequested = false;
    int total = remotePaths.size();
    int successCount = 0;

    for (int i = 0; i < total; ++i) {
        if (m_cancelRequested) {
            emit transferComplete(false, QString("已取消，完成 %1/%2 个文件").arg(successCount).arg(total));
            m_isTransferring = false;
            return false;
        }

        emit fileTransferStarted(QFileInfo(remotePaths[i]).fileName());
        emit batchProgressChanged(i + 1, total, 0);

        if (m_ftpClient->downloadFile(remotePaths[i], localPaths[i])) {
            successCount++;
        }
        emit batchProgressChanged(i + 1, total, 100);
    }

    m_isTransferring = false;
    QString msg = QString("完成 %1/%2 个文件").arg(successCount).arg(total);
    emit transferComplete(successCount == total, msg);
    return successCount == total;
}

bool FileTransfer::localToFtpBatch(const QStringList &localPaths, const QStringList &remotePaths)
{
    if (!m_ftpClient || localPaths.size() != remotePaths.size()) {
        emit transferComplete(false, "参数错误");
        return false;
    }

    m_isTransferring = true;
    m_cancelRequested = false;
    int total = localPaths.size();
    int successCount = 0;

    for (int i = 0; i < total; ++i) {
        if (m_cancelRequested) {
            emit transferComplete(false, QString("已取消，完成 %1/%2 个文件").arg(successCount).arg(total));
            m_isTransferring = false;
            return false;
        }

        emit fileTransferStarted(QFileInfo(localPaths[i]).fileName());
        emit batchProgressChanged(i + 1, total, 0);

        if (m_ftpClient->uploadFile(localPaths[i], remotePaths[i])) {
            successCount++;
        }
        emit batchProgressChanged(i + 1, total, 100);
    }

    m_isTransferring = false;
    QString msg = QString("完成 %1/%2 个文件").arg(successCount).arg(total);
    emit transferComplete(successCount == total, msg);
    return successCount == total;
}

bool FileTransfer::machineToLocalBatch(const QStringList &remotePaths, const QStringList &localPaths)
{
    if (!m_machineFtpClient || remotePaths.size() != localPaths.size()) {
        emit transferComplete(false, "参数错误");
        return false;
    }

    m_isTransferring = true;
    m_cancelRequested = false;
    int total = remotePaths.size();
    int successCount = 0;

    for (int i = 0; i < total; ++i) {
        if (m_cancelRequested) {
            emit transferComplete(false, QString("已取消，完成 %1/%2 个文件").arg(successCount).arg(total));
            m_isTransferring = false;
            return false;
        }

        emit fileTransferStarted(QFileInfo(remotePaths[i]).fileName());
        emit batchProgressChanged(i + 1, total, 0);

        if (m_machineFtpClient->downloadFile(remotePaths[i], localPaths[i])) {
            successCount++;
        }
        emit batchProgressChanged(i + 1, total, 100);
    }

    m_isTransferring = false;
    QString msg = QString("完成 %1/%2 个文件").arg(successCount).arg(total);
    emit transferComplete(successCount == total, msg);
    return successCount == total;
}

bool FileTransfer::localToMachineBatch(const QStringList &localPaths, const QStringList &remotePaths)
{
    if (!m_machineFtpClient || localPaths.size() != remotePaths.size()) {
        emit transferComplete(false, "参数错误");
        return false;
    }

    m_isTransferring = true;
    m_cancelRequested = false;
    int total = localPaths.size();
    int successCount = 0;

    for (int i = 0; i < total; ++i) {
        if (m_cancelRequested) {
            emit transferComplete(false, QString("已取消，完成 %1/%2 个文件").arg(successCount).arg(total));
            m_isTransferring = false;
            return false;
        }

        emit fileTransferStarted(QFileInfo(localPaths[i]).fileName());
        emit batchProgressChanged(i + 1, total, 0);

        if (m_machineFtpClient->uploadFile(localPaths[i], remotePaths[i])) {
            successCount++;
        }
        emit batchProgressChanged(i + 1, total, 100);
    }

    m_isTransferring = false;
    QString msg = QString("完成 %1/%2 个文件").arg(successCount).arg(total);
    emit transferComplete(successCount == total, msg);
    return successCount == total;
}
