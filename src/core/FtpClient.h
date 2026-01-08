#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <QObject>

class QTcpSocket;

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
    bool downloadFile(const QString &remotePath, const QString &localPath);
    bool uploadFile(const QString &localPath, const QString &remotePath);
    bool deleteFile(const QString &remotePath);

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
};

#endif // FTPCLIENT_H
