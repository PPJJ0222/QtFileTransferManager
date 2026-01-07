#include "MainWindow.h"
#include "FilePanel.h"
#include "ProgressBar.h"
#include "../core/FileTransfer.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QMessageBox>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupConnections();
    setWindowTitle("Qt文件传输管理器");
    resize(1200, 700);
}

void MainWindow::setupUI()
{
    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_splitter = new QSplitter(Qt::Horizontal, this);

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

    mainLayout->addWidget(m_splitter);
    mainLayout->addWidget(m_progressBar);

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

    // 连接拖拽信号
    connect(m_ftpPanel, &FilePanel::fileDropped, this, &MainWindow::onFileDropped);
    connect(m_localPanel, &FilePanel::fileDropped, this, &MainWindow::onFileDropped);
    connect(m_machinePanel, &FilePanel::fileDropped, this, &MainWindow::onFileDropped);

    // 设置传输管理器的客户端
    m_fileTransfer->setFtpClient(m_ftpPanel->ftpClient());
    m_fileTransfer->setMachineClient(m_machinePanel->machineClient());
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
    if (success) {
        // 刷新本地面板
        m_localPanel->refresh();
    }
}

void MainWindow::onFileDropped(FilePanel *source, FilePanel *target, const QString &fileName)
{
    using PT = FilePanel::PanelType;
    PT srcType = source->panelType();
    PT dstType = target->panelType();

    QString srcPath = source->currentPath();
    QString dstPath = target->currentPath();

    // 构建完整路径
    QString srcFile, dstFile;
    if (srcType == PT::Local) {
        srcFile = QDir(srcPath).absoluteFilePath(fileName);
    } else {
        srcFile = srcPath.isEmpty() ? fileName : srcPath + "/" + fileName;
    }

    if (dstType == PT::Local) {
        dstFile = QDir(dstPath).absoluteFilePath(fileName);
    } else {
        dstFile = dstPath.isEmpty() ? fileName : dstPath + "/" + fileName;
    }

    m_progressBar->setStatus(QString("正在传输: %1").arg(fileName));

    // 执行传输
    bool success = false;
    if (srcType == PT::FTP && dstType == PT::Local) {
        success = m_fileTransfer->ftpToLocal(srcFile, dstFile);
    } else if (srcType == PT::Local && dstType == PT::FTP) {
        success = m_fileTransfer->localToFtp(srcFile, dstFile);
    } else if (srcType == PT::Machine && dstType == PT::Local) {
        success = m_fileTransfer->machineToLocal(srcFile, dstFile);
    } else if (srcType == PT::Local && dstType == PT::Machine) {
        success = m_fileTransfer->localToMachine(srcFile, dstFile);
    } else {
        QMessageBox::warning(this, "传输错误", "不支持的传输路径。\nFTP和机床之间不能直接传输，请先传输到本地。");
    }
}
