#include "FileTransfer.h"
#include "FtpClient.h"

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
