#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>

class QProgressBar;
class QLabel;

class ProgressBar : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressBar(QWidget *parent = nullptr);

public slots:
    void setProgress(int value);
    void setStatus(const QString &status);
    void reset();

private:
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
};

#endif // PROGRESSBAR_H
