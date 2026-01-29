#include "FtpWorker.h"
#include "core/FtpClient.h"

FtpWorker::FtpWorker(QObject *parent)
    : QObject(parent)
    , m_ftpClient(nullptr)
{
    // FtpClient 在工作线程中创建，确保 QTcpSocket 在正确的线程
    m_ftpClient = new FtpClient(this);

    // 转发错误信号
    connect(m_ftpClient, &FtpClient::error, this, &FtpWorker::error);
}

FtpWorker::~FtpWorker()
{
    if (m_ftpClient && m_ftpClient->isConnected()) {
        m_ftpClient->disconnect();
    }
}

void FtpWorker::connectToServer(const QString &host, int port,
                                 const QString &user, const QString &password)
{
    bool success = m_ftpClient->connectToServer(host, port);
    if (success) {
        success = m_ftpClient->login(user, password);
    }

    if (success) {
        emit connected();
    } else {
        emit error("连接失败");
    }
}

void FtpWorker::disconnectFromServer()
{
    m_ftpClient->disconnect();
    emit disconnected();
}

void FtpWorker::listDirectory(const QString &path)
{
    auto files = m_ftpClient->listFilesWithInfo(path);
    QVariantList result;
    for (const auto &f : files) {
        QVariantMap item;
        item["name"] = f.name;
        item["isDir"] = f.isDir;
        item["size"] = 0;
        result.append(item);
    }
    emit directoryListed(result);
}

void FtpWorker::downloadFile(const QString &remotePath, const QString &localPath)
{
    emit progressChanged(0);
    bool success = m_ftpClient->downloadFile(remotePath, localPath);
    emit progressChanged(100);
    emit downloadCompleted(success, success ? "下载完成" : "下载失败");
}

void FtpWorker::uploadFile(const QString &localPath, const QString &remotePath)
{
    emit progressChanged(0);
    bool success = m_ftpClient->uploadFile(localPath, remotePath);
    emit progressChanged(100);
    emit uploadCompleted(success, success ? "上传完成" : "上传失败");
}

void FtpWorker::downloadDirectory(const QString &remotePath, const QString &localPath)
{
    emit progressChanged(0);
    bool success = m_ftpClient->downloadDirectory(remotePath, localPath);
    emit progressChanged(100);
    emit downloadCompleted(success, success ? "下载完成" : "下载失败");
}

void FtpWorker::uploadDirectory(const QString &localPath, const QString &remotePath)
{
    emit progressChanged(0);
    bool success = m_ftpClient->uploadDirectory(localPath, remotePath);
    emit progressChanged(100);
    emit uploadCompleted(success, success ? "上传完成" : "上传失败");
}

void FtpWorker::cancelCurrentTransfer()
{
    if (m_ftpClient) {
        m_ftpClient->requestCancel();
    }
}
