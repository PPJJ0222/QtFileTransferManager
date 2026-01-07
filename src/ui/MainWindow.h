#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "FilePanel.h"

class QSplitter;
class ProgressBar;
class FileTransfer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onTransferProgress(int percent);
    void onTransferComplete(bool success, const QString &message);
    void onFileDropped(FilePanel *source, FilePanel *target, const QString &fileName);

private:
    void setupUI();
    void setupConnections();

    QSplitter *m_splitter;
    FilePanel *m_ftpPanel;
    FilePanel *m_localPanel;
    FilePanel *m_machinePanel;
    ProgressBar *m_progressBar;
    FileTransfer *m_fileTransfer;
};

#endif // MAINWINDOW_H
