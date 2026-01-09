#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>

class QProgressBar;
class QLabel;
class QPushButton;

class ProgressBar : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressBar(QWidget *parent = nullptr);

public slots:
    void setProgress(int value);
    void setStatus(const QString &status);
    void setBatchProgress(int current, int total, int filePercent);
    void setTransferring(bool transferring);
    void reset();

signals:
    void cancelRequested();

private:
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_percentLabel;
    QPushButton *m_cancelBtn;
};

#endif // PROGRESSBAR_H
