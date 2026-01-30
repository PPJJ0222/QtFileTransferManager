#ifndef BRIDGE_H
#define BRIDGE_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>
#include <QThread>

class FtpWorker;

// C++ 与 JavaScript 通信桥梁
class Bridge : public QObject
{
    Q_OBJECT

public:
    explicit Bridge(QObject *parent = nullptr);
    ~Bridge();

public slots:
    // FTP 操作
    void connectFtp(const QString &host, int port,
                    const QString &user, const QString &password);
    void disconnectFtp();
    void listFtpDirectory(const QString &path);

    // 本地文件操作
    void listLocalDirectory(const QString &path);
    void deleteLocalFiles(const QString &basePath, const QStringList &fileNames);

    // 机床操作
    void connectMachine(const QString &host, int port,
                        const QString &user, const QString &password);
    void disconnectMachine();
    void listMachineDirectory(const QString &path);

    // 文件传输
    void transferFile(const QString &sourcePath, const QString &destPath,
                      const QString &sourceType, const QString &destType);
    void transferDirectory(const QString &sourcePath, const QString &destPath,
                           const QString &sourceType, const QString &destType);
    void cancelTransfer();

    // 配置管理
    void loadFtpServers();
    void loadMachineServers();
    void saveMachineServer(const QVariantMap &config);
    void updateMachineServer(const QString &originalName, const QVariantMap &config);
    void deleteMachineServer(const QString &name);

signals:
    // FTP 信号
    void ftpConnected();
    void ftpDisconnected();
    void ftpError(const QString &error);
    void ftpDirectoryListed(const QVariantList &files);

    // 本地文件信号
    void localDirectoryListed(const QVariantList &files);
    void localFilesDeleted(bool success, const QString &message);

    // 机床信号
    void machineConnected();
    void machineDisconnected();
    void machineError(const QString &error);
    void machineDirectoryListed(const QVariantList &files);

    // 传输信号
    void transferProgress(int current, int total, int percentage,
                          const QString &fileName);
    void transferCompleted(bool success, const QString &message);
    void transferError(const QString &error);

    // 配置信号
    void ftpServersLoaded(const QVariantList &servers);
    void machineServersLoaded(const QVariantList &servers);

private:
    // FTP 工作线程
    QThread *m_ftpThread;
    FtpWorker *m_ftpWorker;

    // 机床工作线程
    QThread *m_machineThread;
    FtpWorker *m_machineWorker;

    bool m_ftpConnected = false;
    bool m_machineConnected = false;
};

#endif // BRIDGE_H
