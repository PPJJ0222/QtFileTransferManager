#include "Bridge.h"
#include "FtpWorker.h"
#include <QSettings>
#include <QDir>
#include <QCoreApplication>

Bridge::Bridge(QObject *parent)
    : QObject(parent)
    , m_ftpThread(new QThread(this))
    , m_ftpWorker(new FtpWorker())
    , m_machineThread(new QThread(this))
    , m_machineWorker(new FtpWorker())
{
    // 将 worker 移动到工作线程
    m_ftpWorker->moveToThread(m_ftpThread);
    m_machineWorker->moveToThread(m_machineThread);

    // FTP worker 信号连接
    connect(m_ftpWorker, &FtpWorker::connected, this, [this]() {
        m_ftpConnected = true;
        emit ftpConnected();
    });
    connect(m_ftpWorker, &FtpWorker::disconnected, this, [this]() {
        m_ftpConnected = false;
        emit ftpDisconnected();
    });
    connect(m_ftpWorker, &FtpWorker::directoryListed, this, &Bridge::ftpDirectoryListed);
    connect(m_ftpWorker, &FtpWorker::error, this, &Bridge::ftpError);
    connect(m_ftpWorker, &FtpWorker::downloadCompleted, this, &Bridge::transferCompleted);
    connect(m_ftpWorker, &FtpWorker::uploadCompleted, this, &Bridge::transferCompleted);
    connect(m_ftpWorker, &FtpWorker::progressChanged, this, [this](int percent) {
        emit transferProgress(1, 1, percent, "");
    });

    // 机床 worker 信号连接
    connect(m_machineWorker, &FtpWorker::connected, this, [this]() {
        m_machineConnected = true;
        emit machineConnected();
    });
    connect(m_machineWorker, &FtpWorker::disconnected, this, [this]() {
        m_machineConnected = false;
        emit machineDisconnected();
    });
    connect(m_machineWorker, &FtpWorker::directoryListed, this, &Bridge::machineDirectoryListed);
    connect(m_machineWorker, &FtpWorker::error, this, &Bridge::machineError);
    connect(m_machineWorker, &FtpWorker::downloadCompleted, this, &Bridge::transferCompleted);
    connect(m_machineWorker, &FtpWorker::uploadCompleted, this, &Bridge::transferCompleted);

    // 线程结束时清理 worker
    connect(m_ftpThread, &QThread::finished, m_ftpWorker, &QObject::deleteLater);
    connect(m_machineThread, &QThread::finished, m_machineWorker, &QObject::deleteLater);

    // 启动工作线程
    m_ftpThread->start();
    m_machineThread->start();
}

Bridge::~Bridge()
{
    // 停止工作线程
    m_ftpThread->quit();
    m_ftpThread->wait();
    m_machineThread->quit();
    m_machineThread->wait();
}

// ========== FTP 操作 ==========

void Bridge::connectFtp(const QString &host, int port,
                        const QString &user, const QString &password)
{
    QMetaObject::invokeMethod(m_ftpWorker, "connectToServer",
                              Q_ARG(QString, host), Q_ARG(int, port),
                              Q_ARG(QString, user), Q_ARG(QString, password));
}

void Bridge::disconnectFtp()
{
    QMetaObject::invokeMethod(m_ftpWorker, "disconnectFromServer");
}

void Bridge::listFtpDirectory(const QString &path)
{
    QMetaObject::invokeMethod(m_ftpWorker, "listDirectory", Q_ARG(QString, path));
}

// ========== 本地文件操作 ==========

void Bridge::listLocalDirectory(const QString &path)
{
    QVariantList result;
    QDir dir(path);
    auto entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const auto &f : entries) {
        QVariantMap item;
        item["name"] = f.fileName();
        item["isDir"] = f.isDir();
        item["size"] = f.size();
        result.append(item);
    }
    emit localDirectoryListed(result);
}

void Bridge::deleteLocalFiles(const QString &basePath, const QStringList &fileNames)
{
    QStringList failedFiles;

    for (const QString &fileName : fileNames) {
        QString fullPath = basePath + "/" + fileName;
        QFileInfo fileInfo(fullPath);

        if (!fileInfo.exists()) {
            failedFiles.append(fileName);
            continue;
        }

        bool success = false;
        if (fileInfo.isDir()) {
            QDir dir(fullPath);
            success = dir.removeRecursively();
        } else {
            success = QFile::remove(fullPath);
        }

        if (!success) {
            failedFiles.append(fileName);
        }
    }

    if (failedFiles.isEmpty()) {
        emit localFilesDeleted(true, QString("成功删除 %1 个项目").arg(fileNames.size()));
    } else {
        emit localFilesDeleted(false, QString("删除失败: %1").arg(failedFiles.join(", ")));
    }
}

// ========== 机床操作 ==========

void Bridge::connectMachine(const QString &host, int port,
                            const QString &user, const QString &password)
{
    QMetaObject::invokeMethod(m_machineWorker, "connectToServer",
                              Q_ARG(QString, host), Q_ARG(int, port),
                              Q_ARG(QString, user), Q_ARG(QString, password));
}

void Bridge::disconnectMachine()
{
    QMetaObject::invokeMethod(m_machineWorker, "disconnectFromServer");
}

void Bridge::listMachineDirectory(const QString &path)
{
    QMetaObject::invokeMethod(m_machineWorker, "listDirectory", Q_ARG(QString, path));
}

// ========== 文件传输 ==========

void Bridge::transferFile(const QString &sourcePath, const QString &destPath,
                          const QString &sourceType, const QString &destType)
{
    if (sourceType == "ftp" && destType == "local") {
        QMetaObject::invokeMethod(m_ftpWorker, "downloadFile",
                                  Q_ARG(QString, sourcePath), Q_ARG(QString, destPath));
    } else if (sourceType == "local" && destType == "ftp") {
        QMetaObject::invokeMethod(m_ftpWorker, "uploadFile",
                                  Q_ARG(QString, sourcePath), Q_ARG(QString, destPath));
    } else if (sourceType == "machine" && destType == "local") {
        QMetaObject::invokeMethod(m_machineWorker, "downloadFile",
                                  Q_ARG(QString, sourcePath), Q_ARG(QString, destPath));
    } else if (sourceType == "local" && destType == "machine") {
        QMetaObject::invokeMethod(m_machineWorker, "uploadFile",
                                  Q_ARG(QString, sourcePath), Q_ARG(QString, destPath));
    }
}

void Bridge::cancelTransfer()
{
    // 请求 FTP worker 取消
    if (m_ftpWorker) {
        QMetaObject::invokeMethod(m_ftpWorker, "cancelCurrentTransfer", Qt::QueuedConnection);
    }
    // 请求机床 worker 取消
    if (m_machineWorker) {
        QMetaObject::invokeMethod(m_machineWorker, "cancelCurrentTransfer", Qt::QueuedConnection);
    }
}

void Bridge::transferDirectory(const QString &sourcePath, const QString &destPath,
                               const QString &sourceType, const QString &destType)
{
    if (sourceType == "ftp" && destType == "local") {
        QMetaObject::invokeMethod(m_ftpWorker, "downloadDirectory",
                                  Q_ARG(QString, sourcePath), Q_ARG(QString, destPath));
    } else if (sourceType == "local" && destType == "ftp") {
        QMetaObject::invokeMethod(m_ftpWorker, "uploadDirectory",
                                  Q_ARG(QString, sourcePath), Q_ARG(QString, destPath));
    } else if (sourceType == "machine" && destType == "local") {
        QMetaObject::invokeMethod(m_machineWorker, "downloadDirectory",
                                  Q_ARG(QString, sourcePath), Q_ARG(QString, destPath));
    } else if (sourceType == "local" && destType == "machine") {
        QMetaObject::invokeMethod(m_machineWorker, "uploadDirectory",
                                  Q_ARG(QString, sourcePath), Q_ARG(QString, destPath));
    }
}

// ========== 配置管理 ==========

void Bridge::loadFtpServers()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/ftp_servers.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    int count = settings.value("Servers/count", 0).toInt();
    QVariantList servers;
    for (int i = 0; i < count; ++i) {
        QString prefix = QString("Server_%1/").arg(i);
        QVariantMap server;
        server["name"] = settings.value(prefix + "name").toString();
        server["host"] = settings.value(prefix + "host").toString();
        server["port"] = settings.value(prefix + "port", 21).toInt();
        server["user"] = settings.value(prefix + "user").toString();
        server["password"] = settings.value(prefix + "password").toString();
        servers.append(server);
    }
    emit ftpServersLoaded(servers);
}

void Bridge::loadMachineServers()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/machine_servers.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    int count = settings.value("Servers/count", 0).toInt();
    QVariantList servers;
    for (int i = 0; i < count; ++i) {
        QString prefix = QString("Server_%1/").arg(i);
        QVariantMap server;
        server["name"] = settings.value(prefix + "name").toString();
        server["host"] = settings.value(prefix + "host").toString();
        server["port"] = settings.value(prefix + "port", 21).toInt();
        server["user"] = settings.value(prefix + "user").toString();
        server["password"] = settings.value(prefix + "password").toString();
        servers.append(server);
    }
    emit machineServersLoaded(servers);
}

void Bridge::saveMachineServer(const QVariantMap &config)
{
    QString configPath = QCoreApplication::applicationDirPath() + "/machine_servers.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    int count = settings.value("Servers/count", 0).toInt();
    QString prefix = QString("Server_%1/").arg(count);
    settings.setValue(prefix + "name", config["name"]);
    settings.setValue(prefix + "host", config["host"]);
    settings.setValue(prefix + "port", config["port"]);
    settings.setValue(prefix + "user", config["user"]);
    settings.setValue(prefix + "password", config["password"]);
    settings.setValue("Servers/count", count + 1);
}

void Bridge::deleteMachineServer(const QString &name)
{
    QString configPath = QCoreApplication::applicationDirPath() + "/machine_servers.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    int count = settings.value("Servers/count", 0).toInt();

    // 查找并删除指定服务器
    for (int i = 0; i < count; ++i) {
        QString prefix = QString("Server_%1/").arg(i);
        if (settings.value(prefix + "name").toString() == name) {
            // 将后面的服务器前移
            for (int j = i; j < count - 1; ++j) {
                QString from = QString("Server_%1/").arg(j + 1);
                QString to = QString("Server_%1/").arg(j);
                settings.setValue(to + "name", settings.value(from + "name"));
                settings.setValue(to + "host", settings.value(from + "host"));
                settings.setValue(to + "port", settings.value(from + "port"));
                settings.setValue(to + "user", settings.value(from + "user"));
                settings.setValue(to + "password", settings.value(from + "password"));
            }
            settings.setValue("Servers/count", count - 1);
            break;
        }
    }
}
