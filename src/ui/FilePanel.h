#ifndef FILEPANEL_H
#define FILEPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QMimeData>
#include <QDrag>
#include <QComboBox>
#include <QFutureWatcher>

class QLineEdit;
class QPushButton;
class QLabel;
class QMenu;
class FtpClient;
class FilePanel;

// 自定义列表控件，支持拖拽时设置自定义MIME数据
class DragListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit DragListWidget(FilePanel *panel, QWidget *parent = nullptr);
protected:
    void startDrag(Qt::DropActions supportedActions) override;
private:
    FilePanel *m_panel;
};

class FilePanel : public QWidget
{
    Q_OBJECT

public:
    enum class PanelType { FTP, Local, Machine };

    // FTP服务器配置结构体
    struct FtpServerConfig {
        QString name;
        QString host;
        int port;
        QString user;
        QString password;
    };

    explicit FilePanel(PanelType type, QWidget *parent = nullptr);

    PanelType panelType() const { return m_type; }
    QString currentPath() const;
    QString selectedFilePath() const;
    QString selectedFileName() const;
    QStringList selectedFileNames() const;
    FtpClient* ftpClient() const { return m_ftpClient; }
    FtpClient* machineFtpClient() const { return m_machineFtpClient; }

public slots:
    void refresh();
    void navigateTo(const QString &path);
    bool connectFtp(const QString &host, int port, const QString &user, const QString &password);
    void disconnectFtp();
    bool connectMachine(const QString &host, int port, const QString &user, const QString &password);
    void disconnectMachine();
    void reloadServers();

signals:
    void fileSelected(const QString &filePath);
    void pathChanged(const QString &newPath);
    void filesDropped(FilePanel *source, FilePanel *target, const QStringList &fileNames);

private slots:
    void onItemDoubleClicked();
    void onPathEditFinished();
    void onGoUp();
    void showContextMenu(const QPoint &pos);
    void onDeleteFile();
    void onRenameFile();
    void onCreateFolder();
    void onConnectFtp();
    void onConnectMachine();
    void onServerChanged(int index);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUI();
    void updateFileList();
    void loadFtpServers();
    void loadMachineServers();
    void setLoading(bool loading);

    PanelType m_type;
    QString m_currentPath;

    QLabel *m_titleLabel;
    QLineEdit *m_pathEdit;
    QPushButton *m_refreshBtn;
    QPushButton *m_upBtn;
    QPushButton *m_connectBtn;
    QComboBox *m_serverCombo;
    DragListWidget *m_fileList;
    QMenu *m_contextMenu;
    FtpClient *m_ftpClient;
    FtpClient *m_machineFtpClient;
    QList<FtpServerConfig> m_ftpServers;
    QList<FtpServerConfig> m_machineServers;
    bool m_isLoading = false;
    QFutureWatcher<bool> *m_connectWatcher = nullptr;
    FtpServerConfig m_pendingConfig;
};

#endif // FILEPANEL_H
