#include "FtpClient.h"
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QRegularExpression>
#include <QFile>

FtpClient::FtpClient(QObject *parent)
    : QObject(parent)
    , m_controlSocket(new QTcpSocket(this))
    , m_port(21)
{
}

FtpClient::~FtpClient()
{
    disconnect();
}

bool FtpClient::connectToServer(const QString &host, int port)
{
    m_host = host;
    m_port = port;

    m_controlSocket->connectToHost(host, port);
    if (!m_controlSocket->waitForConnected(5000)) {
        emit error("连接服务器失败");
        return false;
    }

    // 读取欢迎消息 (220)
    QString response = readResponse();
    if (!response.startsWith("220")) {
        emit error("服务器响应异常: " + response);
        return false;
    }
    return true;
}

bool FtpClient::login(const QString &user, const QString &password)
{
    // 发送用户名
    if (!sendCommand("USER " + user)) return false;
    QString response = readResponse();
    if (!response.startsWith("331")) {
        emit error("用户名错误: " + response);
        return false;
    }

    // 发送密码
    if (!sendCommand("PASS " + password)) return false;
    response = readResponse();
    if (!response.startsWith("230")) {
        emit error("密码错误: " + response);
        return false;
    }

    // 设置二进制模式
    sendCommand("TYPE I");
    readResponse();
    return true;
}

bool FtpClient::isConnected() const
{
    return m_controlSocket && m_controlSocket->state() == QAbstractSocket::ConnectedState;
}

void FtpClient::disconnect()
{
    if (m_controlSocket->state() == QAbstractSocket::ConnectedState) {
        sendCommand("QUIT");
        m_controlSocket->disconnectFromHost();
    }
}

QStringList FtpClient::listFiles(const QString &path)
{
    // 切换目录
    if (!path.isEmpty()) {
        sendCommand("CWD " + path);
        QString response = readResponse();
        if (!response.startsWith("250")) {
            emit error("切换目录失败: " + response);
            return {};
        }
    }

    // 进入被动模式
    QString dataHost;
    int dataPort;
    if (!enterPassiveMode(dataHost, dataPort)) return {};

    // 建立数据连接
    QTcpSocket dataSocket;
    dataSocket.connectToHost(dataHost, dataPort);
    if (!dataSocket.waitForConnected(5000)) {
        emit error("数据连接失败");
        return {};
    }

    // 发送LIST命令
    if (!sendCommand("LIST")) return {};
    QString response = readResponse();
    if (!response.startsWith("150") && !response.startsWith("125")) {
        emit error("LIST命令失败: " + response);
        return {};
    }

    // 读取目录列表
    QByteArray data;
    while (dataSocket.waitForReadyRead(3000)) {
        data.append(dataSocket.readAll());
    }
    dataSocket.close();

    // 等待传输完成响应
    readResponse();

    // 解析文件列表 (Unix格式: drwxr-xr-x ... filename)
    QStringList files;
    QString listing = QString::fromUtf8(data);
    for (const QString &line : listing.split('\n', Qt::SkipEmptyParts)) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;
        // 取最后一个空格后的内容作为文件名
        int lastSpace = trimmed.lastIndexOf(' ');
        if (lastSpace > 0) {
            files.append(trimmed.mid(lastSpace + 1));
        }
    }
    return files;
}

bool FtpClient::downloadFile(const QString &remotePath, const QString &localPath)
{
    // 进入被动模式
    QString dataHost;
    int dataPort;
    if (!enterPassiveMode(dataHost, dataPort)) return false;

    // 建立数据连接
    QTcpSocket dataSocket;
    dataSocket.connectToHost(dataHost, dataPort);
    if (!dataSocket.waitForConnected(5000)) {
        emit error("数据连接失败");
        return false;
    }

    // 发送RETR命令
    if (!sendCommand("RETR " + remotePath)) return false;
    QString response = readResponse();
    if (!response.startsWith("150") && !response.startsWith("125")) {
        emit error("下载命令失败: " + response);
        return false;
    }

    // 接收数据并写入本地文件
    QFile file(localPath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error("无法创建本地文件");
        return false;
    }

    while (dataSocket.waitForReadyRead(5000)) {
        file.write(dataSocket.readAll());
    }
    file.close();
    dataSocket.close();

    // 等待传输完成响应
    response = readResponse();
    return response.startsWith("226");
}

bool FtpClient::uploadFile(const QString &localPath, const QString &remotePath)
{
    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("无法打开本地文件");
        return false;
    }

    // 进入被动模式
    QString dataHost;
    int dataPort;
    if (!enterPassiveMode(dataHost, dataPort)) {
        file.close();
        return false;
    }

    // 建立数据连接
    QTcpSocket dataSocket;
    dataSocket.connectToHost(dataHost, dataPort);
    if (!dataSocket.waitForConnected(5000)) {
        emit error("数据连接失败");
        file.close();
        return false;
    }

    // 发送STOR命令
    if (!sendCommand("STOR " + remotePath)) {
        file.close();
        return false;
    }
    QString response = readResponse();
    if (!response.startsWith("150") && !response.startsWith("125")) {
        emit error("上传命令失败: " + response);
        file.close();
        return false;
    }

    // 发送文件数据
    while (!file.atEnd()) {
        dataSocket.write(file.read(8192));
        dataSocket.waitForBytesWritten(5000);
    }
    file.close();
    dataSocket.close();

    // 等待传输完成响应
    response = readResponse();
    return response.startsWith("226");
}

bool FtpClient::deleteFile(const QString &remotePath)
{
    if (!sendCommand("DELE " + remotePath)) return false;
    QString response = readResponse();
    return response.startsWith("250");
}

bool FtpClient::sendCommand(const QString &cmd)
{
    if (m_controlSocket->state() != QAbstractSocket::ConnectedState)
        return false;
    m_controlSocket->write((cmd + "\r\n").toUtf8());
    return m_controlSocket->waitForBytesWritten(5000);
}

QString FtpClient::readResponse()
{
    if (m_controlSocket->waitForReadyRead(5000)) {
        return QString::fromUtf8(m_controlSocket->readAll());
    }
    return {};
}

bool FtpClient::enterPassiveMode(QString &host, int &port)
{
    if (!sendCommand("PASV")) return false;
    QString response = readResponse();
    if (!response.startsWith("227")) {
        emit error("进入被动模式失败: " + response);
        return false;
    }

    // 解析 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
    QRegularExpression re(R"(\((\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\))");
    auto match = re.match(response);
    if (!match.hasMatch()) {
        emit error("解析被动模式响应失败");
        return false;
    }

    host = QString("%1.%2.%3.%4")
        .arg(match.captured(1), match.captured(2), match.captured(3), match.captured(4));
    port = match.captured(5).toInt() * 256 + match.captured(6).toInt();
    return true;
}
