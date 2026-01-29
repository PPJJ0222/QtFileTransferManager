#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <QObject>
#include <QList>
#include <atomic>

class QTcpSocket;

// FTP文件信息结构体
struct FtpFileInfo {
    QString name;
    bool isDir;
};

class FtpClient : public QObject
{
    Q_OBJECT

public:
    explicit FtpClient(QObject *parent = nullptr);
    ~FtpClient();

    bool connectToServer(const QString &host, int port = 21);
    bool login(const QString &user, const QString &password);
    bool isConnected() const;
    void disconnect();

    QStringList listFiles(const QString &path);
    QList<FtpFileInfo> listFilesWithInfo(const QString &path);
    QString currentDirectory();
    bool downloadFile(const QString &remotePath, const QString &localPath);
    bool uploadFile(const QString &localPath, const QString &remotePath);
    bool deleteFile(const QString &remotePath);

    // 目录操作
    bool createDirectory(const QString &remotePath);
    bool downloadDirectory(const QString &remotePath, const QString &localPath);
    bool uploadDirectory(const QString &localPath, const QString &remotePath);

    // 取消传输
    void requestCancel() { m_cancelRequested = true; }
    void resetCancel() { m_cancelRequested = false; }

signals:
    void progressChanged(int percent);
    void error(const QString &message);

private:
    bool sendCommand(const QString &cmd);
    QString readResponse();
    bool enterPassiveMode(QString &host, int &port);

    QTcpSocket *m_controlSocket;
    QString m_host;
    int m_port;
    std::atomic<bool> m_cancelRequested{false};
};

#endif // FTPCLIENT_H
