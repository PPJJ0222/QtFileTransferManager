#ifndef MACHINECLIENT_H
#define MACHINECLIENT_H

#include <QObject>
#include <QStringList>

// 机床客户端抽象基类
class MachineClient : public QObject
{
    Q_OBJECT

public:
    explicit MachineClient(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~MachineClient() = default;

    virtual bool connectToMachine(const QString &host, int port) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual QStringList listFiles(const QString &path) = 0;
    virtual bool downloadFile(const QString &remotePath, const QString &localPath) = 0;
    virtual bool uploadFile(const QString &localPath, const QString &remotePath) = 0;
    virtual bool deleteFile(const QString &remotePath) = 0;

signals:
    void progressChanged(int percent);
    void error(const QString &message);
    void connected();
    void disconnected();
};

// TCP协议实现
class MachineTcpClient : public MachineClient
{
    Q_OBJECT

public:
    explicit MachineTcpClient(QObject *parent = nullptr);
    ~MachineTcpClient() override;

    bool connectToMachine(const QString &host, int port) override;
    void disconnect() override;
    bool isConnected() const override;

    QStringList listFiles(const QString &path) override;
    bool downloadFile(const QString &remotePath, const QString &localPath) override;
    bool uploadFile(const QString &localPath, const QString &remotePath) override;
    bool deleteFile(const QString &remotePath) override;

private:
    class Impl;
    Impl *m_impl;
};

#endif // MACHINECLIENT_H
