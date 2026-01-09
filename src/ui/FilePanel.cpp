#include "FilePanel.h"
#include "StyleManager.h"
#include "../core/LocalFileOps.h"
#include "../core/FtpClient.h"
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
#include <QFileInfo>
#include <QDebug>
#include <QFrame>
#include <QTimer>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QTcpSocket>

// DragListWidget实现
DragListWidget::DragListWidget(FilePanel *panel, QWidget *parent)
    : QListWidget(parent), m_panel(panel)
{
}

void DragListWidget::startDrag(Qt::DropActions supportedActions)
{
    QList<QListWidgetItem*> items = selectedItems();
    if (items.isEmpty()) return;

    // 收集所有选中的文件名（排除".."）
    QStringList fileNames;
    for (auto *item : items) {
        if (item->text() != "..") {
            fileNames.append(item->text());
        }
    }
    if (fileNames.isEmpty()) return;

    auto *drag = new QDrag(m_panel);
    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-filepanel-item", fileNames.join("\n").toUtf8());
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}

FilePanel::FilePanel(PanelType type, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_ftpClient(nullptr)
    , m_machineFtpClient(nullptr)
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

    // 机床面板创建FTP客户端
    if (m_type == PanelType::Machine) {
        m_machineFtpClient = new FtpClient(this);
    }
}

void FilePanel::setupUI()
{
    // 设置面板objectName用于QSS
    setObjectName("FilePanel");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === 标题栏 ===
    auto *headerWidget = new QWidget(this);
    headerWidget->setObjectName("PanelHeader");
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(12, 10, 12, 10);

    // 标题图标
    auto *iconLabel = new QLabel(this);
    iconLabel->setFixedSize(20, 20);
    switch (m_type) {
    case PanelType::FTP:
        iconLabel->setPixmap(StyleManager::buttonIcon("server").pixmap(20, 20));
        break;
    case PanelType::Local:
        iconLabel->setPixmap(StyleManager::buttonIcon("local").pixmap(20, 20));
        break;
    case PanelType::Machine:
        iconLabel->setPixmap(StyleManager::buttonIcon("machine").pixmap(20, 20));
        break;
    }
    headerLayout->addWidget(iconLabel);

    // 标题文字
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");
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
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    mainLayout->addWidget(headerWidget);

    // === 工具栏 ===
    auto *toolbarWidget = new QWidget(this);
    toolbarWidget->setObjectName("Toolbar");
    auto *toolbarLayout = new QVBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(8, 8, 8, 8);
    toolbarLayout->setSpacing(6);

    // 第一行：路径栏
    auto *pathLayout = new QHBoxLayout();
    pathLayout->setSpacing(6);

    m_upBtn = new QPushButton(this);
    m_upBtn->setObjectName("toolButton");
    m_upBtn->setIcon(StyleManager::buttonIcon("up"));
    m_upBtn->setToolTip("返回上级目录");
    m_upBtn->setFixedSize(28, 28);

    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setPlaceholderText("路径...");

    m_refreshBtn = new QPushButton(this);
    m_refreshBtn->setObjectName("toolButton");
    m_refreshBtn->setIcon(StyleManager::buttonIcon("refresh"));
    m_refreshBtn->setToolTip("刷新");
    m_refreshBtn->setFixedSize(28, 28);

    pathLayout->addWidget(m_upBtn);
    pathLayout->addWidget(m_pathEdit, 1);
    pathLayout->addWidget(m_refreshBtn);
    toolbarLayout->addLayout(pathLayout);

    // 第二行：服务器选择（FTP和机床面板）
    if (m_type == PanelType::FTP || m_type == PanelType::Machine) {
        auto *serverLayout = new QHBoxLayout();
        serverLayout->setSpacing(6);

        if (m_type == PanelType::FTP) {
            loadFtpServers();
        } else {
            loadMachineServers();
        }

        m_serverCombo = new QComboBox(this);
        const auto &servers = (m_type == PanelType::FTP) ? m_ftpServers : m_machineServers;
        for (const auto &server : servers) {
            m_serverCombo->addItem(server.name);
        }
        serverLayout->addWidget(m_serverCombo, 1);

        m_connectBtn = new QPushButton("连接", this);
        m_connectBtn->setIcon(StyleManager::buttonIcon("connect"));
        m_connectBtn->setMinimumWidth(80);
        serverLayout->addWidget(m_connectBtn);

        if (m_type == PanelType::FTP) {
            connect(m_connectBtn, &QPushButton::clicked, this, &FilePanel::onConnectFtp);
        } else {
            connect(m_connectBtn, &QPushButton::clicked, this, &FilePanel::onConnectMachine);
        }

        toolbarLayout->addLayout(serverLayout);
    }

    mainLayout->addWidget(toolbarWidget);

    // === 文件列表 ===
    m_fileList = new DragListWidget(this, this);
    m_fileList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_fileList->setDragEnabled(true);
    m_fileList->setDragDropMode(QAbstractItemView::DragOnly);
    m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_fileList->setAlternatingRowColors(true);
    mainLayout->addWidget(m_fileList, 1);

    // 右键菜单（仅本地面板）
    m_contextMenu = new QMenu(this);
    if (m_type == PanelType::Local) {
        auto *createAction = m_contextMenu->addAction(StyleManager::icon("add"), "新建文件夹");
        connect(createAction, &QAction::triggered, this, &FilePanel::onCreateFolder);

        auto *renameAction = m_contextMenu->addAction(StyleManager::icon("edit"), "重命名");
        connect(renameAction, &QAction::triggered, this, &FilePanel::onRenameFile);

        auto *deleteAction = m_contextMenu->addAction(StyleManager::icon("delete"), "删除");
        connect(deleteAction, &QAction::triggered, this, &FilePanel::onDeleteFile);
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

QStringList FilePanel::selectedFileNames() const
{
    QStringList names;
    for (auto *item : m_fileList->selectedItems()) {
        if (item->text() != "..") {
            names.append(item->text());
        }
    }
    return names;
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
    } else if (m_type == PanelType::Machine && m_machineFtpClient) {
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
    } else if (m_type == PanelType::Machine && m_machineFtpClient && m_machineFtpClient->isConnected()) {
        // 添加返回上级目录项
        if (!m_currentPath.isEmpty()) {
            m_fileList->addItem("..");
        }

        // 获取机床文件列表（使用FTP）
        QStringList files = m_machineFtpClient->listFiles(m_currentPath);
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
        if (m_connectBtn) {
            m_connectBtn->setText("断开");
            m_connectBtn->setIcon(StyleManager::buttonIcon("disconnect"));
        }
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
        if (m_connectBtn) {
            m_connectBtn->setText("连接");
            m_connectBtn->setIcon(StyleManager::buttonIcon("connect"));
        }
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

    // 显示loading状态
    setLoading(true);
    m_pendingConfig = m_ftpServers[index];

    // 清理旧的watcher
    if (m_connectWatcher) {
        m_connectWatcher->deleteLater();
    }
    m_connectWatcher = new QFutureWatcher<bool>(this);

    connect(m_connectWatcher, &QFutureWatcher<bool>::finished, this, [this]() {
        bool canConnect = m_connectWatcher->result();
        if (canConnect) {
            // 在主线程中执行实际连接
            bool success = connectFtp(m_pendingConfig.host, m_pendingConfig.port,
                                      m_pendingConfig.user, m_pendingConfig.password);
            setLoading(false);
            if (!success) {
                QMessageBox::warning(this, "错误", QString("连接 %1 失败").arg(m_pendingConfig.name));
            }
        } else {
            setLoading(false);
            QMessageBox::warning(this, "错误", QString("无法连接到 %1").arg(m_pendingConfig.name));
        }
    });

    // 在后台线程测试连接
    QString host = m_pendingConfig.host;
    int port = m_pendingConfig.port;
    QFuture<bool> future = QtConcurrent::run([host, port]() {
        QTcpSocket socket;
        socket.connectToHost(host, port);
        bool connected = socket.waitForConnected(5000);
        if (connected) socket.disconnectFromHost();
        return connected;
    });
    m_connectWatcher->setFuture(future);
}

bool FilePanel::connectMachine(const QString &host, int port, const QString &user, const QString &password)
{
    if (!m_machineFtpClient) return false;

    if (m_machineFtpClient->connectToServer(host, port) && m_machineFtpClient->login(user, password)) {
        m_currentPath = "";
        updateFileList();
        if (m_connectBtn) {
            m_connectBtn->setText("断开");
            m_connectBtn->setIcon(StyleManager::buttonIcon("disconnect"));
        }
        return true;
    }
    return false;
}

void FilePanel::disconnectMachine()
{
    if (m_machineFtpClient) {
        m_machineFtpClient->disconnect();
        m_fileList->clear();
        m_currentPath = "";
        m_pathEdit->clear();
        if (m_connectBtn) {
            m_connectBtn->setText("连接");
            m_connectBtn->setIcon(StyleManager::buttonIcon("connect"));
        }
    }
}

void FilePanel::onConnectMachine()
{
    if (!m_machineFtpClient) return;

    // 如果已连接，则断开
    if (m_connectBtn && m_connectBtn->text() == "断开") {
        disconnectMachine();
        return;
    }

    // 获取选中的服务器配置
    int index = m_serverCombo ? m_serverCombo->currentIndex() : -1;
    if (index < 0 || index >= m_machineServers.size()) {
        QMessageBox::warning(this, "错误", "请选择机床服务器");
        return;
    }

    // 显示loading状态
    setLoading(true);
    m_pendingConfig = m_machineServers[index];

    // 清理旧的watcher
    if (m_connectWatcher) {
        m_connectWatcher->deleteLater();
    }
    m_connectWatcher = new QFutureWatcher<bool>(this);

    connect(m_connectWatcher, &QFutureWatcher<bool>::finished, this, [this]() {
        bool canConnect = m_connectWatcher->result();
        if (canConnect) {
            // 在主线程中执行实际连接
            bool success = connectMachine(m_pendingConfig.host, m_pendingConfig.port,
                                          m_pendingConfig.user, m_pendingConfig.password);
            setLoading(false);
            if (!success) {
                QMessageBox::warning(this, "错误", QString("连接机床 %1 失败").arg(m_pendingConfig.name));
            }
        } else {
            setLoading(false);
            QMessageBox::warning(this, "错误", QString("无法连接到机床 %1").arg(m_pendingConfig.name));
        }
    });

    // 在后台线程测试连接
    QString host = m_pendingConfig.host;
    int port = m_pendingConfig.port;
    QFuture<bool> future = QtConcurrent::run([host, port]() {
        QTcpSocket socket;
        socket.connectToHost(host, port);
        bool connected = socket.waitForConnected(5000);
        if (connected) socket.disconnectFromHost();
        return connected;
    });
    m_connectWatcher->setFuture(future);
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

    QString data = event->mimeData()->data("application/x-filepanel-item");
    QStringList fileNames = data.split("\n", Qt::SkipEmptyParts);
    if (!fileNames.isEmpty()) {
        emit filesDropped(sourcePanel, this, fileNames);
        event->acceptProposedAction();
    }
}

void FilePanel::loadFtpServers()
{
    // Try common locations so the INI works both in the build tree and next to the app binary
    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        QDir(appDir).filePath("ftp_servers.ini"),
        QDir(appDir).absoluteFilePath("../ftp_servers.ini"),
        QDir(QDir::currentPath()).filePath("ftp_servers.ini"),
        QDir(appDir).filePath("../Resources/ftp_servers.ini") // macOS .app bundle
    };

    QString configPath;
    for (const auto &path : candidates) {
        if (QFileInfo::exists(path)) {
            configPath = QFileInfo(path).absoluteFilePath();
            break;
        }
    }

    if (configPath.isEmpty()) {
        qWarning() << "ftp_servers.ini not found, searched:" << candidates;
        return;
    }

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

void FilePanel::loadMachineServers()
{
    // Try common locations so the INI works both in the build tree and next to the app binary
    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        QDir(appDir).filePath("machine_servers.ini"),
        QDir(appDir).absoluteFilePath("../machine_servers.ini"),
        QDir(QDir::currentPath()).filePath("machine_servers.ini"),
        QDir(appDir).filePath("../Resources/machine_servers.ini") // macOS .app bundle
    };

    QString configPath;
    for (const auto &path : candidates) {
        if (QFileInfo::exists(path)) {
            configPath = QFileInfo(path).absoluteFilePath();
            break;
        }
    }

    if (configPath.isEmpty()) {
        qWarning() << "machine_servers.ini not found, searched:" << candidates;
        return;
    }

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
        m_machineServers.append(config);
    }
}

void FilePanel::reloadServers()
{
    if (m_type == PanelType::Machine && m_serverCombo) {
        m_machineServers.clear();
        loadMachineServers();
        m_serverCombo->clear();
        for (const auto &server : m_machineServers) {
            m_serverCombo->addItem(server.name);
        }
    }
}

void FilePanel::setLoading(bool loading)
{
    m_isLoading = loading;
    if (m_connectBtn) {
        m_connectBtn->setEnabled(!loading);
        m_connectBtn->setText(loading ? "连接中..." : "连接");
    }
    if (m_serverCombo) {
        m_serverCombo->setEnabled(!loading);
    }
    m_fileList->setEnabled(!loading);
    if (loading) {
        m_fileList->clear();
        m_fileList->addItem("正在连接服务器...");
    } else {
        m_fileList->clear();
    }
}
