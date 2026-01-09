#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QObject>
#include <QStringList>

class FtpClient;

class FileTransfer : public QObject
{
    Q_OBJECT

public:
    explicit FileTransfer(QObject *parent = nullptr);

    void setFtpClient(FtpClient *client) { m_ftpClient = client; }
    void setMachineFtpClient(FtpClient *client) { m_machineFtpClient = client; }

    // FTP <-> 本地（单文件）
    bool ftpToLocal(const QString &remotePath, const QString &localPath);
    bool localToFtp(const QString &localPath, const QString &remotePath);

    // 机床 <-> 本地（单文件）
    bool machineToLocal(const QString &remotePath, const QString &localPath);
    bool localToMachine(const QString &localPath, const QString &remotePath);

    // 批量传输方法
    bool ftpToLocalBatch(const QStringList &remotePaths, const QStringList &localPaths);
    bool localToFtpBatch(const QStringList &localPaths, const QStringList &remotePaths);
    bool machineToLocalBatch(const QStringList &remotePaths, const QStringList &localPaths);
    bool localToMachineBatch(const QStringList &localPaths, const QStringList &remotePaths);

    // 取消传输
    void cancelTransfer();
    bool isTransferring() const { return m_isTransferring; }

signals:
    void progressChanged(int percent);
    void transferComplete(bool success, const QString &message);
    void batchProgressChanged(int current, int total, int filePercent);
    void fileTransferStarted(const QString &fileName);

private:
    FtpClient *m_ftpClient;
    FtpClient *m_machineFtpClient;
    bool m_cancelRequested = false;
    bool m_isTransferring = false;
};

#endif // FILETRANSFER_H
