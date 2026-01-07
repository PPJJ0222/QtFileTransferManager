#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QObject>

class FtpClient;
class MachineClient;

class FileTransfer : public QObject
{
    Q_OBJECT

public:
    explicit FileTransfer(QObject *parent = nullptr);

    void setFtpClient(FtpClient *client) { m_ftpClient = client; }
    void setMachineClient(MachineClient *client) { m_machineClient = client; }

    // FTP <-> 本地
    bool ftpToLocal(const QString &remotePath, const QString &localPath);
    bool localToFtp(const QString &localPath, const QString &remotePath);

    // 机床 <-> 本地
    bool machineToLocal(const QString &remotePath, const QString &localPath);
    bool localToMachine(const QString &localPath, const QString &remotePath);

signals:
    void progressChanged(int percent);
    void transferComplete(bool success, const QString &message);

private:
    FtpClient *m_ftpClient;
    MachineClient *m_machineClient;
};

#endif // FILETRANSFER_H
