#include "ProgressBar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>

ProgressBar::ProgressBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("ProgressArea");

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(12);

    // 状态标签
    m_statusLabel = new QLabel("就绪", this);
    m_statusLabel->setObjectName("StatusLabel");
    m_statusLabel->setMinimumWidth(200);
    mainLayout->addWidget(m_statusLabel);

    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(18);
    mainLayout->addWidget(m_progressBar, 1);

    // 百分比标签
    m_percentLabel = new QLabel("0%", this);
    m_percentLabel->setFixedWidth(60);
    m_percentLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mainLayout->addWidget(m_percentLabel);

    // 取消按钮
    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setFixedWidth(60);
    m_cancelBtn->hide();
    connect(m_cancelBtn, &QPushButton::clicked, this, &ProgressBar::cancelRequested);
    mainLayout->addWidget(m_cancelBtn);
}

void ProgressBar::setProgress(int value)
{
    m_progressBar->setValue(value);
    m_percentLabel->setText(QString("%1%").arg(value));
}

void ProgressBar::setStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void ProgressBar::setBatchProgress(int current, int total, int filePercent)
{
    // 计算总体进度
    int overallPercent = ((current - 1) * 100 + filePercent) / total;
    m_progressBar->setValue(overallPercent);
    m_percentLabel->setText(QString("%1/%2").arg(current).arg(total));
}

void ProgressBar::setTransferring(bool transferring)
{
    m_cancelBtn->setVisible(transferring);
    if (!transferring) {
        m_progressBar->setValue(0);
    }
}

void ProgressBar::reset()
{
    m_progressBar->setValue(0);
    m_percentLabel->setText("0%");
    m_statusLabel->setText("就绪");
    m_cancelBtn->hide();
}
