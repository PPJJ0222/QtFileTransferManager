#include "FilePanel.h"
#include "../core/LocalFileOps.h"
#include "../core/FtpClient.h"
#include "../core/MachineClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QDir>
#include <QFileIconProvider>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDrag>
#include <QSettings>
#include <QCoreApplication>

// DragListWidget实现
DragListWidget::DragListWidget(FilePanel *panel, QWidget *parent)
    : QListWidget(parent), m_panel(panel)
{
}

void DragListWidget::startDrag(Qt::DropActions supportedActions)
{
    auto *item = currentItem();
    if (!item || item->text() == "..") return;

    auto *drag = new QDrag(m_panel);
    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-filepanel-item", item->text().toUtf8());
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}

FilePanel::FilePanel(PanelType type, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_ftpClient(nullptr)
    , m_machineClient(nullptr)
    , m_connectBtn(nullptr)
    , m_serverCombo(nullptr)
{
    setupUI();
    setAcceptDrops(true);

    // 本地面板默认显示用户主目录
    if (m_type == PanelType::Local) {
        m_currentPath = QDir::homePath();
        updateFileList();
    }

    // FTP面板创建客户端
    if (m_type == PanelType::FTP) {
        m_ftpClient = new FtpClient(this);
    }

    // 机床面板创建客户端
    if (m_type == PanelType::Machine) {
        m_machineClient = new MachineTcpClient(this);
    }
}

void FilePanel::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);

    // 标题
    m_titleLabel = new QLabel(this);
    switch (m_type) {
    case PanelType::FTP:
        m_titleLabel->setText("FTP服务器");
        break;
    case PanelType::Local:
        m_titleLabel->setText("本地文件");
        break;
    case PanelType::Machine:
        m_titleLabel->setText("机床设备");
        break;
    }
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(m_titleLabel);

    // 路径栏
    auto *pathLayout = new QHBoxLayout();
    m_upBtn = new QPushButton("↑", this);
    m_upBtn->setFixedWidth(30);
    m_pathEdit = new QLineEdit(this);
    m_refreshBtn = new QPushButton("刷新", this);

    pathLayout->addWidget(m_upBtn);
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(m_refreshBtn);

    // FTP面板添加服务器选择下拉框和连接按钮
    if (m_type == PanelType::FTP) {
        loadFtpServers();

        m_serverCombo = new QComboBox(this);
        for (const auto &server : m_ftpServers) {
            m_serverCombo->addItem(server.name);
        }
        pathLayout->addWidget(m_serverCombo);

        m_connectBtn = new QPushButton("连接", this);
        pathLayout->addWidget(m_connectBtn);
        connect(m_connectBtn, &QPushButton::clicked, this, &FilePanel::onConnectFtp);
    }

    // 机床面板添加连接按钮
    if (m_type == PanelType::Machine) {
        m_connectBtn = new QPushButton("连接", this);
        pathLayout->addWidget(m_connectBtn);
        connect(m_connectBtn, &QPushButton::clicked, this, &FilePanel::onConnectMachine);
    }

    layout->addLayout(pathLayout);

    // 文件列表
    m_fileList = new DragListWidget(this, this);
    m_fileList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_fileList->setDragEnabled(true);
    m_fileList->setDragDropMode(QAbstractItemView::DragOnly);
    m_fileList->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_fileList);

    // 右键菜单（仅本地面板）
    m_contextMenu = new QMenu(this);
    if (m_type == PanelType::Local) {
        m_contextMenu->addAction("新建文件夹", this, &FilePanel::onCreateFolder);
        m_contextMenu->addAction("重命名", this, &FilePanel::onRenameFile);
        m_contextMenu->addAction("删除", this, &FilePanel::onDeleteFile);
    }

    // 连接信号
    connect(m_refreshBtn, &QPushButton::clicked, this, &FilePanel::refresh);
    connect(m_upBtn, &QPushButton::clicked, this, &FilePanel::onGoUp);
    connect(m_pathEdit, &QLineEdit::returnPressed, this, &FilePanel::onPathEditFinished);
    connect(m_fileList, &QListWidget::itemDoubleClicked, this, &FilePanel::onItemDoubleClicked);
    connect(m_fileList, &QListWidget::customContextMenuRequested, this, &FilePanel::showContextMenu);
}

QString FilePanel::currentPath() const
{
    return m_currentPath;
}

QString FilePanel::selectedFilePath() const
{
    auto *item = m_fileList->currentItem();
    if (!item || item->text() == "..") return {};
    QDir dir(m_currentPath);
    return dir.absoluteFilePath(item->text());
}

QString FilePanel::selectedFileName() const
{
    auto *item = m_fileList->currentItem();
    if (!item || item->text() == "..") return {};
    return item->text();
}

void FilePanel::refresh()
{
    updateFileList();
}

void FilePanel::navigateTo(const QString &path)
{
    m_currentPath = path;
    m_pathEdit->setText(path);
    updateFileList();
    emit pathChanged(path);
}

void FilePanel::onItemDoubleClicked()
{
    auto *item = m_fileList->currentItem();
    if (!item) return;

    QString name = item->text();
    if (name == "..") {
        onGoUp();
        return;
    }

    if (m_type == PanelType::Local) {
        QDir dir(m_currentPath);
        QString fullPath = dir.absoluteFilePath(name);
        QFileInfo info(fullPath);
        if (info.isDir()) {
            navigateTo(fullPath);
        } else {
            emit fileSelected(fullPath);
        }
    } else if (m_type == PanelType::FTP && m_ftpClient) {
        QString newPath = m_currentPath.isEmpty() ? name : m_currentPath + "/" + name;
        navigateTo(newPath);
    } else if (m_type == PanelType::Machine && m_machineClient) {
        QString newPath = m_currentPath.isEmpty() ? name : m_currentPath + "/" + name;
        navigateTo(newPath);
    }
}

void FilePanel::onPathEditFinished()
{
    QString path = m_pathEdit->text();
    if (m_type == PanelType::Local) {
        QDir dir(path);
        if (dir.exists()) {
            navigateTo(path);
        }
    } else if (m_type == PanelType::FTP || m_type == PanelType::Machine) {
        navigateTo(path);
    }
}

void FilePanel::onGoUp()
{
    if (m_type == PanelType::Local) {
        QDir dir(m_currentPath);
        if (dir.cdUp()) {
            navigateTo(dir.absolutePath());
        }
    } else if (m_type == PanelType::FTP || m_type == PanelType::Machine) {
        int lastSlash = m_currentPath.lastIndexOf('/');
        if (lastSlash > 0) {
            navigateTo(m_currentPath.left(lastSlash));
        } else if (!m_currentPath.isEmpty()) {
            navigateTo("");
        }
    }
}

void FilePanel::updateFileList()
{
    m_fileList->clear();
    m_pathEdit->setText(m_currentPath);

    if (m_type == PanelType::Local && !m_currentPath.isEmpty()) {
        QDir dir(m_currentPath);
        QFileIconProvider iconProvider;

        // 添加返回上级目录项
        if (!dir.isRoot()) {
            m_fileList->addItem("..");
        }

        // 获取目录和文件列表
        auto entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot,
                                         QDir::DirsFirst | QDir::Name);
        for (const auto &entry : entries) {
            auto *item = new QListWidgetItem(iconProvider.icon(entry), entry.fileName());
            m_fileList->addItem(item);
        }
    } else if (m_type == PanelType::FTP && m_ftpClient) {
        // 添加返回上级目录项
        if (!m_currentPath.isEmpty()) {
            m_fileList->addItem("..");
        }

        // 获取FTP文件列表
        QStringList files = m_ftpClient->listFiles(m_currentPath);
        for (const QString &file : files) {
            m_fileList->addItem(file);
        }
    } else if (m_type == PanelType::Machine && m_machineClient && m_machineClient->isConnected()) {
        // 添加返回上级目录项
        if (!m_currentPath.isEmpty()) {
            m_fileList->addItem("..");
        }

        // 获取机床文件列表
        QStringList files = m_machineClient->listFiles(m_currentPath);
        for (const QString &file : files) {
            m_fileList->addItem(file);
        }
    }
}

void FilePanel::showContextMenu(const QPoint &pos)
{
    if (m_type != PanelType::Local) return;
    m_contextMenu->exec(m_fileList->mapToGlobal(pos));
}

void FilePanel::onDeleteFile()
{
    QString path = selectedFilePath();
    if (path.isEmpty()) return;

    QFileInfo info(path);
    QString msg = info.isDir()
        ? QString("确定删除文件夹 \"%1\" 及其所有内容?").arg(info.fileName())
        : QString("确定删除文件 \"%1\"?").arg(info.fileName());

    if (QMessageBox::question(this, "确认删除", msg) == QMessageBox::Yes) {
        if (LocalFileOps::deleteFile(path)) {
            refresh();
        } else {
            QMessageBox::warning(this, "错误", "删除失败");
        }
    }
}

void FilePanel::onRenameFile()
{
    QString path = selectedFilePath();
    if (path.isEmpty()) return;

    QFileInfo info(path);
    QString newName = QInputDialog::getText(this, "重命名", "新名称:",
                                            QLineEdit::Normal, info.fileName());
    if (newName.isEmpty() || newName == info.fileName()) return;

    QString newPath = info.dir().absoluteFilePath(newName);
    if (LocalFileOps::rename(path, newPath)) {
        refresh();
    } else {
        QMessageBox::warning(this, "错误", "重命名失败");
    }
}

void FilePanel::onCreateFolder()
{
    QString name = QInputDialog::getText(this, "新建文件夹", "文件夹名称:");
    if (name.isEmpty()) return;

    QDir dir(m_currentPath);
    QString newPath = dir.absoluteFilePath(name);
    if (LocalFileOps::createDirectory(newPath)) {
        refresh();
    } else {
        QMessageBox::warning(this, "错误", "创建文件夹失败");
    }
}

bool FilePanel::connectFtp(const QString &host, int port, const QString &user, const QString &password)
{
    if (!m_ftpClient) return false;

    if (m_ftpClient->connectToServer(host, port) && m_ftpClient->login(user, password)) {
        m_currentPath = "";
        updateFileList();
        if (m_connectBtn) m_connectBtn->setText("断开");
        return true;
    }
    return false;
}

void FilePanel::disconnectFtp()
{
    if (m_ftpClient) {
        m_ftpClient->disconnect();
        m_fileList->clear();
        m_currentPath = "";
        m_pathEdit->clear();
        if (m_connectBtn) m_connectBtn->setText("连接");
    }
}

void FilePanel::onConnectFtp()
{
    if (!m_ftpClient) return;

    // 如果已连接，则断开
    if (m_connectBtn && m_connectBtn->text() == "断开") {
        disconnectFtp();
        return;
    }

    // 获取选中的服务器配置
    int index = m_serverCombo ? m_serverCombo->currentIndex() : -1;
    if (index < 0 || index >= m_ftpServers.size()) {
        QMessageBox::warning(this, "错误", "请选择服务器");
        return;
    }

    const auto &config = m_ftpServers[index];
    if (!connectFtp(config.host, config.port, config.user, config.password)) {
        QMessageBox::warning(this, "错误", QString("连接 %1 失败").arg(config.name));
    }
}

bool FilePanel::connectMachine(const QString &host, int port)
{
    if (!m_machineClient) return false;

    if (m_machineClient->connectToMachine(host, port)) {
        m_currentPath = "";
        updateFileList();
        if (m_connectBtn) m_connectBtn->setText("断开");
        return true;
    }
    return false;
}

void FilePanel::disconnectMachine()
{
    if (m_machineClient) {
        m_machineClient->disconnect();
        m_fileList->clear();
        m_currentPath = "";
        m_pathEdit->clear();
        if (m_connectBtn) m_connectBtn->setText("连接");
    }
}

void FilePanel::onConnectMachine()
{
    if (!m_machineClient) return;

    // 如果已连接，则断开
    if (m_machineClient->isConnected()) {
        disconnectMachine();
        return;
    }

    // 显示连接对话框
    QDialog dialog(this);
    dialog.setWindowTitle("机床连接");
    auto *layout = new QFormLayout(&dialog);

    auto *hostEdit = new QLineEdit(&dialog);
    hostEdit->setText("192.168.1.100");
    auto *portEdit = new QLineEdit(&dialog);
    portEdit->setText("8080");

    layout->addRow("主机:", hostEdit);
    layout->addRow("端口:", portEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (!connectMachine(hostEdit->text(), portEdit->text().toInt())) {
            QMessageBox::warning(this, "错误", "机床连接失败");
        }
    }
}

void FilePanel::dragEnterEvent(QDragEnterEvent *event)
{
    // 只接受来自其他FilePanel的拖拽
    if (event->mimeData()->hasFormat("application/x-filepanel-item")) {
        // 检查传输路径是否有效（本地必须作为中转）
        auto *sourcePanel = qobject_cast<FilePanel*>(event->source());
        if (sourcePanel && sourcePanel != this) {
            bool validTransfer = (sourcePanel->panelType() == PanelType::Local ||
                                  m_type == PanelType::Local);
            if (validTransfer) {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void FilePanel::dropEvent(QDropEvent *event)
{
    auto *sourcePanel = qobject_cast<FilePanel*>(event->source());
    if (!sourcePanel || sourcePanel == this) {
        event->ignore();
        return;
    }

    QString fileName = event->mimeData()->data("application/x-filepanel-item");
    if (!fileName.isEmpty()) {
        emit fileDropped(sourcePanel, this, fileName);
        event->acceptProposedAction();
    }
}

void FilePanel::loadFtpServers()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/ftp_servers.ini";
    QSettings settings(configPath, QSettings::IniFormat);

    int count = settings.value("Servers/count", 0).toInt();
    for (int i = 0; i < count; ++i) {
        QString prefix = QString("Server_%1/").arg(i);
        FtpServerConfig config;
        config.name = settings.value(prefix + "name").toString();
        config.host = settings.value(prefix + "host").toString();
        config.port = settings.value(prefix + "port", 21).toInt();
        config.user = settings.value(prefix + "user").toString();
        config.password = settings.value(prefix + "password").toString();
        m_ftpServers.append(config);
    }
}
