#ifndef MACHINECONFIGDIALOG_H
#define MACHINECONFIGDIALOG_H

#include <QDialog>
#include <QVector>

class QListWidget;
class QPushButton;

struct MachineServerConfig {
    QString name;
    QString host;
    int port = 21;
    QString user;
    QString password;
};

class MachineConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MachineConfigDialog(QWidget *parent = nullptr);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onSave();

private:
    void loadServers();
    void updateList();
    bool editServer(MachineServerConfig &config, bool isNew);
    QString getConfigPath() const;

    QListWidget *m_listWidget;
    QPushButton *m_addBtn;
    QPushButton *m_editBtn;
    QPushButton *m_deleteBtn;
    QVector<MachineServerConfig> m_servers;
};

#endif // MACHINECONFIGDIALOG_H
