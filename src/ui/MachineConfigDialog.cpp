#include "MachineConfigDialog.h"
#include "StyleManager.h"
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

MachineConfigDialog::MachineConfigDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("机床服务器配置");
  resize(480, 360);
  setMinimumSize(400, 300);

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // === 标题栏 ===
  auto *headerWidget = new QWidget(this);
  headerWidget->setObjectName("PanelHeader");
  auto *headerLayout = new QHBoxLayout(headerWidget);
  headerLayout->setContentsMargins(16, 12, 16, 12);

  auto *iconLabel = new QLabel(this);
  iconLabel->setPixmap(StyleManager::icon("machine").pixmap(24, 24));
  headerLayout->addWidget(iconLabel);

  auto *titleLabel = new QLabel("机床服务器配置", this);
  titleLabel->setObjectName("PanelTitle");
  headerLayout->addWidget(titleLabel);
  headerLayout->addStretch();

  mainLayout->addWidget(headerWidget);

  // === 内容区域 ===
  auto *contentWidget = new QWidget(this);
  auto *contentLayout = new QHBoxLayout(contentWidget);
  contentLayout->setContentsMargins(16, 16, 16, 16);
  contentLayout->setSpacing(12);

  // 左侧列表
  m_listWidget = new QListWidget(this);
  m_listWidget->setAlternatingRowColors(true);
  contentLayout->addWidget(m_listWidget, 1);

  // 右侧按钮区
  auto *btnWidget = new QWidget(this);
  auto *btnLayout = new QVBoxLayout(btnWidget);
  btnLayout->setContentsMargins(0, 0, 0, 0);
  btnLayout->setSpacing(8);

  m_addBtn = new QPushButton("添加", this);
  m_addBtn->setIcon(StyleManager::buttonIcon("add"));
  m_editBtn = new QPushButton("编辑", this);
  m_editBtn->setIcon(StyleManager::buttonIcon("edit"));
  m_deleteBtn = new QPushButton("删除", this);
  m_deleteBtn->setIcon(StyleManager::buttonIcon("delete"));
  m_deleteBtn->setProperty("danger", true);

  auto *saveBtn = new QPushButton("保存", this);
  saveBtn->setIcon(StyleManager::buttonIcon("save"));
  auto *cancelBtn = new QPushButton("取消", this);
  cancelBtn->setIcon(StyleManager::buttonIcon("cancel"));
  cancelBtn->setProperty("secondary", true);

  btnLayout->addWidget(m_addBtn);
  btnLayout->addSpacing(2);
  btnLayout->addWidget(m_editBtn);
  btnLayout->addSpacing(2);
  btnLayout->addWidget(m_deleteBtn);
  btnLayout->addStretch();

  // 分隔线
  auto *separator = new QFrame(this);
  separator->setFrameShape(QFrame::HLine);
  separator->setFrameShadow(QFrame::Sunken);
  btnLayout->addWidget(separator);
  btnLayout->addSpacing(8);

  btnLayout->addWidget(saveBtn);
  btnLayout->addSpacing(2);
  btnLayout->addWidget(cancelBtn);

  contentLayout->addWidget(btnWidget);
  mainLayout->addWidget(contentWidget, 1);

  connect(m_addBtn, &QPushButton::clicked, this, &MachineConfigDialog::onAdd);
  connect(m_editBtn, &QPushButton::clicked, this, &MachineConfigDialog::onEdit);
  connect(m_deleteBtn, &QPushButton::clicked, this,
          &MachineConfigDialog::onDelete);
  connect(saveBtn, &QPushButton::clicked, this, &MachineConfigDialog::onSave);
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

  loadServers();
  updateList();
}

QString MachineConfigDialog::getConfigPath() const {
  const QString appDir = QCoreApplication::applicationDirPath();
  QStringList candidates = {
      QDir(appDir).filePath("machine_servers.ini"),
      QDir(appDir).absoluteFilePath("../machine_servers.ini"),
      QDir(QDir::currentPath()).filePath("machine_servers.ini")};

  for (const auto &path : candidates) {
    if (QFileInfo::exists(path)) {
      return QFileInfo(path).absoluteFilePath();
    }
  }
  // 默认返回当前目录
  return QDir(QDir::currentPath()).filePath("machine_servers.ini");
}

void MachineConfigDialog::loadServers() {
  m_servers.clear();
  QString configPath = getConfigPath();
  if (!QFileInfo::exists(configPath))
    return;

  QSettings settings(configPath, QSettings::IniFormat);
  int count = settings.value("Servers/count", 0).toInt();
  for (int i = 0; i < count; ++i) {
    QString prefix = QString("Server_%1/").arg(i);
    MachineServerConfig config;
    config.name = settings.value(prefix + "name").toString();
    config.host = settings.value(prefix + "host").toString();
    config.port = settings.value(prefix + "port", 21).toInt();
    config.user = settings.value(prefix + "user").toString();
    config.password = settings.value(prefix + "password").toString();
    m_servers.append(config);
  }
}

void MachineConfigDialog::updateList() {
  m_listWidget->clear();
  for (const auto &server : m_servers) {
    m_listWidget->addItem(server.name);
  }
}

bool MachineConfigDialog::editServer(MachineServerConfig &config, bool isNew) {
  QDialog dlg(this);
  dlg.setWindowTitle(isNew ? "添加服务器" : "编辑服务器");

  auto *layout = new QFormLayout(&dlg);
  auto *nameEdit = new QLineEdit(config.name, &dlg);
  auto *hostEdit = new QLineEdit(config.host, &dlg);
  auto *portSpin = new QSpinBox(&dlg);
  portSpin->setRange(1, 65535);
  portSpin->setValue(config.port);
  auto *userEdit = new QLineEdit(config.user, &dlg);
  auto *passEdit = new QLineEdit(config.password, &dlg);
  passEdit->setEchoMode(QLineEdit::Password);

  layout->addRow("名称:", nameEdit);
  layout->addRow("主机:", hostEdit);
  layout->addRow("端口:", portSpin);
  layout->addRow("用户名:", userEdit);
  layout->addRow("密码:", passEdit);

  auto *btnBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
  btnBox->button(QDialogButtonBox::Ok)->setText("确认");
  btnBox->button(QDialogButtonBox::Cancel)->setText("取消");
  layout->addRow(btnBox);
  connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
  connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

  if (dlg.exec() == QDialog::Accepted) {
    config.name = nameEdit->text();
    config.host = hostEdit->text();
    config.port = portSpin->value();
    config.user = userEdit->text();
    config.password = passEdit->text();
    return true;
  }
  return false;
}

void MachineConfigDialog::onAdd() {
  MachineServerConfig config;
  config.port = 21;
  if (editServer(config, true)) {
    m_servers.append(config);
    updateList();
  }
}

void MachineConfigDialog::onEdit() {
  int row = m_listWidget->currentRow();
  if (row < 0 || row >= m_servers.size())
    return;

  if (editServer(m_servers[row], false)) {
    updateList();
  }
}

void MachineConfigDialog::onDelete() {
  int row = m_listWidget->currentRow();
  if (row < 0 || row >= m_servers.size())
    return;

  m_servers.removeAt(row);
  updateList();
}

void MachineConfigDialog::onSave() {
  QString configPath = getConfigPath();
  QSettings settings(configPath, QSettings::IniFormat);
  settings.clear();

  settings.setValue("Servers/count", m_servers.size());
  for (int i = 0; i < m_servers.size(); ++i) {
    QString prefix = QString("Server_%1/").arg(i);
    settings.setValue(prefix + "name", m_servers[i].name);
    settings.setValue(prefix + "host", m_servers[i].host);
    settings.setValue(prefix + "port", m_servers[i].port);
    settings.setValue(prefix + "user", m_servers[i].user);
    settings.setValue(prefix + "password", m_servers[i].password);
  }
  settings.sync();
  accept();
}
