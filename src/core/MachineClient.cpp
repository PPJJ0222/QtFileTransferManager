#include "MachineClient.h"
#include <QTcpSocket>
#include <QFile>
#include <QDataStream>

// 简单协议命令定义
namespace MachineProtocol {
    constexpr quint8 CMD_LIST = 0x01;
    constexpr quint8 CMD_DOWNLOAD = 0x02;
    constexpr quint8 CMD_UPLOAD = 0x03;
    constexpr quint8 CMD_DELETE = 0x04;
    constexpr quint8 RESP_OK = 0x00;
    constexpr quint8 RESP_ERROR = 0xFF;
}

class MachineTcpClient::Impl
{
public:
    QTcpSocket *socket = nullptr;
    bool connected = false;

    bool sendCommand(quint8 cmd, const QByteArray &data = {}) {
        if (socket->state() != QAbstractSocket::ConnectedState) return false;
        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);
        stream << cmd << static_cast<quint32>(data.size());
        packet.append(data);
        socket->write(packet);
        return socket->waitForBytesWritten(5000);
    }

    QByteArray readResponse() {
        if (!socket->waitForReadyRead(5000)) return {};
        return socket->readAll();
    }
};

MachineTcpClient::MachineTcpClient(QObject *parent)
    : MachineClient(parent)
    , m_impl(new Impl)
{
    m_impl->socket = new QTcpSocket(this);
}

MachineTcpClient::~MachineTcpClient()
{
    disconnect();
    delete m_impl;
}

bool MachineTcpClient::connectToMachine(const QString &host, int port)
{
    m_impl->socket->connectToHost(host, port);
    if (!m_impl->socket->waitForConnected(5000)) {
        emit error("连接机床失败");
        return false;
    }
    m_impl->connected = true;
    emit connected();
    return true;
}

void MachineTcpClient::disconnect()
{
    if (m_impl->socket->state() == QAbstractSocket::ConnectedState) {
        m_impl->socket->disconnectFromHost();
    }
    m_impl->connected = false;
}

bool MachineTcpClient::isConnected() const
{
    return m_impl->connected;
}

QStringList MachineTcpClient::listFiles(const QString &path)
{
    if (!m_impl->connected) return {};

    // 发送LIST命令
    if (!m_impl->sendCommand(MachineProtocol::CMD_LIST, path.toUtf8())) {
        emit error("发送命令失败");
        return {};
    }

    // 读取响应
    QByteArray response = m_impl->readResponse();
    if (response.isEmpty() || static_cast<quint8>(response[0]) == MachineProtocol::RESP_ERROR) {
        emit error("获取文件列表失败");
        return {};
    }

    // 解析文件列表 (以换行符分隔)
    QString data = QString::fromUtf8(response.mid(1));
    return data.split('\n', Qt::SkipEmptyParts);
}

bool MachineTcpClient::downloadFile(const QString &remotePath, const QString &localPath)
{
    if (!m_impl->connected) return false;

    // 发送DOWNLOAD命令
    if (!m_impl->sendCommand(MachineProtocol::CMD_DOWNLOAD, remotePath.toUtf8())) {
        emit error("发送命令失败");
        return false;
    }

    // 读取文件数据
    QByteArray response = m_impl->readResponse();
    if (response.isEmpty() || static_cast<quint8>(response[0]) == MachineProtocol::RESP_ERROR) {
        emit error("下载文件失败");
        return false;
    }

    // 写入本地文件
    QFile file(localPath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error("无法创建本地文件");
        return false;
    }
    file.write(response.mid(1));
    file.close();
    return true;
}

bool MachineTcpClient::uploadFile(const QString &localPath, const QString &remotePath)
{
    if (!m_impl->connected) return false;

    // 读取本地文件
    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("无法打开本地文件");
        return false;
    }
    QByteArray fileData = file.readAll();
    file.close();

    // 构造上传数据: 远程路径 + '\0' + 文件内容
    QByteArray data = remotePath.toUtf8() + '\0' + fileData;

    // 发送UPLOAD命令
    if (!m_impl->sendCommand(MachineProtocol::CMD_UPLOAD, data)) {
        emit error("发送命令失败");
        return false;
    }

    // 读取响应
    QByteArray response = m_impl->readResponse();
    return !response.isEmpty() && response[0] == MachineProtocol::RESP_OK;
}

bool MachineTcpClient::deleteFile(const QString &remotePath)
{
    if (!m_impl->connected) return false;

    // 发送DELETE命令
    if (!m_impl->sendCommand(MachineProtocol::CMD_DELETE, remotePath.toUtf8())) {
        emit error("发送命令失败");
        return false;
    }

    // 读取响应
    QByteArray response = m_impl->readResponse();
    return !response.isEmpty() && response[0] == MachineProtocol::RESP_OK;
}
