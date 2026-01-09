#include "MainWindow.h"
#include "FilePanel.h"
#include "ProgressBar.h"
#include "MachineConfigDialog.h"
#include "StyleManager.h"
#include "../core/FileTransfer.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QMessageBox>
#include <QDir>
#include <QMenuBar>
#include <QMenu>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupConnections();
    setWindowTitle("Qt文件传输管理器");
    resize(1280, 720);
    setMinimumSize(900, 600);
}

void MainWindow::setupUI()
{
    // 创建菜单栏
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu *transferMenu = menuBar->addMenu("文件传输");
    auto *refreshAction = transferMenu->addAction(StyleManager::icon("refresh"), "刷新全部");
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshAll);
    transferMenu->addSeparator();
    transferMenu->addAction("退出", qApp, &QApplication::quit);

    QMenu *configMenu = menuBar->addMenu("机床配置");
    auto *configAction = configMenu->addAction(StyleManager::icon("machine"), "配置机床服务器...");
    connect(configAction, &QAction::triggered, this, &MainWindow::onMachineConfig);

    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setHandleWidth(3);

    m_ftpPanel = new FilePanel(FilePanel::PanelType::FTP, this);
    m_localPanel = new FilePanel(FilePanel::PanelType::Local, this);
    m_machinePanel = new FilePanel(FilePanel::PanelType::Machine, this);

    m_splitter->addWidget(m_ftpPanel);
    m_splitter->addWidget(m_localPanel);
    m_splitter->addWidget(m_machinePanel);

    // 设置初始比例 1:1:1
    m_splitter->setSizes({400, 400, 400});

    // 进度条
    m_progressBar = new ProgressBar(this);

    mainLayout->addWidget(m_splitter, 1);
    mainLayout->addWidget(m_progressBar, 0);

    setCentralWidget(centralWidget);

    // 创建传输管理器
    m_fileTransfer = new FileTransfer(this);
}

void MainWindow::setupConnections()
{
    // 连接传输管理器信号
    connect(m_fileTransfer, &FileTransfer::progressChanged,
            this, &MainWindow::onTransferProgress);
    connect(m_fileTransfer, &FileTransfer::transferComplete,
            this, &MainWindow::onTransferComplete);
    connect(m_fileTransfer, &FileTransfer::batchProgressChanged,
            this, &MainWindow::onBatchProgress);
    connect(m_fileTransfer, &FileTransfer::fileTransferStarted,
            this, &MainWindow::onFileTransferStarted);

    // 连接拖拽信号
    connect(m_ftpPanel, &FilePanel::filesDropped, this, &MainWindow::onFilesDropped);
    connect(m_localPanel, &FilePanel::filesDropped, this, &MainWindow::onFilesDropped);
    connect(m_machinePanel, &FilePanel::filesDropped, this, &MainWindow::onFilesDropped);

    // 连接进度条取消信号
    connect(m_progressBar, &ProgressBar::cancelRequested, this, &MainWindow::onCancelTransfer);

    // 设置传输管理器的客户端
    m_fileTransfer->setFtpClient(m_ftpPanel->ftpClient());
    m_fileTransfer->setMachineFtpClient(m_machinePanel->machineFtpClient());
}

void MainWindow::onTransferProgress(int percent)
{
    m_progressBar->setProgress(percent);
    if (percent > 0 && percent < 100) {
        m_progressBar->setStatus(QString("传输中... %1%").arg(percent));
    }
}

void MainWindow::onTransferComplete(bool success, const QString &message)
{
    m_progressBar->setStatus(message);
    m_progressBar->setTransferring(false);
    if (success) {
        // 刷新本地面板
        m_localPanel->refresh();
    }
}

void MainWindow::onFilesDropped(FilePanel *source, FilePanel *target, const QStringList &fileNames)
{
    using PT = FilePanel::PanelType;
    PT srcType = source->panelType();
    PT dstType = target->panelType();

    QString srcPath = source->currentPath();
    QString dstPath = target->currentPath();

    // 构建完整路径列表
    QStringList srcFiles, dstFiles;
    for (const QString &fileName : fileNames) {
        if (srcType == PT::Local) {
            srcFiles.append(QDir(srcPath).absoluteFilePath(fileName));
        } else {
            srcFiles.append(srcPath.isEmpty() ? fileName : srcPath + "/" + fileName);
        }

        if (dstType == PT::Local) {
            dstFiles.append(QDir(dstPath).absoluteFilePath(fileName));
        } else {
            dstFiles.append(dstPath.isEmpty() ? fileName : dstPath + "/" + fileName);
        }
    }

    m_progressBar->setStatus(QString("正在传输 %1 个文件...").arg(fileNames.size()));
    m_progressBar->setTransferring(true);

    // 执行批量传输
    if (srcType == PT::FTP && dstType == PT::Local) {
        m_fileTransfer->ftpToLocalBatch(srcFiles, dstFiles);
    } else if (srcType == PT::Local && dstType == PT::FTP) {
        m_fileTransfer->localToFtpBatch(srcFiles, dstFiles);
    } else if (srcType == PT::Machine && dstType == PT::Local) {
        m_fileTransfer->machineToLocalBatch(srcFiles, dstFiles);
    } else if (srcType == PT::Local && dstType == PT::Machine) {
        m_fileTransfer->localToMachineBatch(srcFiles, dstFiles);
    } else {
        QMessageBox::warning(this, "传输错误", "不支持的传输路径。\nFTP和机床之间不能直接传输，请先传输到本地。");
        m_progressBar->setTransferring(false);
    }
}

void MainWindow::onBatchProgress(int current, int total, int filePercent)
{
    m_progressBar->setBatchProgress(current, total, filePercent);
}

void MainWindow::onFileTransferStarted(const QString &fileName)
{
    m_progressBar->setStatus(QString("正在传输: %1").arg(fileName));
}

void MainWindow::onCancelTransfer()
{
    m_fileTransfer->cancelTransfer();
}

void MainWindow::onRefreshAll()
{
    m_ftpPanel->refresh();
    m_localPanel->refresh();
    m_machinePanel->refresh();
}

void MainWindow::onMachineConfig()
{
    MachineConfigDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_machinePanel->reloadServers();
    }
}
