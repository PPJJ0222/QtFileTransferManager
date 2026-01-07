#include "ProgressBar.h"
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>

ProgressBar::ProgressBar(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);

    m_statusLabel = new QLabel("就绪", this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);

    layout->addWidget(m_statusLabel);
    layout->addWidget(m_progressBar);
}

void ProgressBar::setProgress(int value)
{
    m_progressBar->setValue(value);
}

void ProgressBar::setStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void ProgressBar::reset()
{
    m_progressBar->setValue(0);
    m_statusLabel->setText("就绪");
}
