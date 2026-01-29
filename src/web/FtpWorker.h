#ifndef FTPWORKER_H
#define FTPWORKER_H

#include <QObject>
#include <QVariantList>

class FtpClient;

// FTP 工作线程类 - 所有 FTP 操作在此线程中执行
class FtpWorker : public QObject
{
    Q_OBJECT

public:
    explicit FtpWorker(QObject *parent = nullptr);
    ~FtpWorker();

public slots:
    // 连接操作
    void connectToServer(const QString &host, int port,
                         const QString &user, const QString &password);
    void disconnectFromServer();

    // 目录操作
    void listDirectory(const QString &path);

    // 文件传输
    void downloadFile(const QString &remotePath, const QString &localPath);
    void uploadFile(const QString &localPath, const QString &remotePath);
    void downloadDirectory(const QString &remotePath, const QString &localPath);
    void uploadDirectory(const QString &localPath, const QString &remotePath);
    void cancelCurrentTransfer();

signals:
    void connected();
    void disconnected();
    void directoryListed(const QVariantList &files);
    void downloadCompleted(bool success, const QString &message);
    void uploadCompleted(bool success, const QString &message);
    void progressChanged(int percent);
    void error(const QString &message);

private:
    FtpClient *m_ftpClient;
};

#endif // FTPWORKER_H
