#ifndef FILEPANEL_H
#define FILEPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QMimeData>
#include <QDrag>

class QLineEdit;
class QPushButton;
class QLabel;
class QMenu;
class FtpClient;
class MachineClient;
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

    explicit FilePanel(PanelType type, QWidget *parent = nullptr);

    PanelType panelType() const { return m_type; }
    QString currentPath() const;
    QString selectedFilePath() const;
    QString selectedFileName() const;
    FtpClient* ftpClient() const { return m_ftpClient; }
    MachineClient* machineClient() const { return m_machineClient; }

public slots:
    void refresh();
    void navigateTo(const QString &path);
    bool connectFtp(const QString &host, int port, const QString &user, const QString &password);
    void disconnectFtp();
    bool connectMachine(const QString &host, int port);
    void disconnectMachine();

signals:
    void fileSelected(const QString &filePath);
    void pathChanged(const QString &newPath);
    void fileDropped(FilePanel *source, FilePanel *target, const QString &fileName);

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

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUI();
    void updateFileList();

    PanelType m_type;
    QString m_currentPath;

    QLabel *m_titleLabel;
    QLineEdit *m_pathEdit;
    QPushButton *m_refreshBtn;
    QPushButton *m_upBtn;
    QPushButton *m_connectBtn;
    DragListWidget *m_fileList;
    QMenu *m_contextMenu;
    FtpClient *m_ftpClient;
    MachineClient *m_machineClient;
};

#endif // FILEPANEL_H
