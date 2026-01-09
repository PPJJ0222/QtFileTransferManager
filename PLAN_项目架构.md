# Qt文件传输工具 - 项目架构

## 项目定位

**小型实用工具** - 专注核心功能,保持简洁

**技术栈**: Qt 6.10.1, C++23, CMake
**目标平台**: Windows
**架构**: UI+Core两层架构

## 核心功能

### 传输拓扑
```
FTP服务器 ←→ 本地文件系统 ←→ 机床设备
```

**重要限制**: FTP和机床**不能直接通信**,本地作为唯一中转站

### 支持的4种传输路径
1. FTP → 本地
2. 本地 → FTP
3. 机床 → 本地
4. 本地 → 机床

### UI布局
- 菜单栏: 文件传输(刷新/退出) + 机床配置
- 三列文件浏览器 (FTP | 本地 | 机床)
- 支持拖拽传输
- 底部进度条

## 项目结构

```
QtFileTransferManager/
├── CMakeLists.txt
├── ftp_servers.ini          # FTP服务器配置
├── machine_servers.ini      # 机床服务器配置
└── src/
    ├── main.cpp
    ├── ui/
    │   ├── MainWindow.h/cpp           # 主窗口 + 菜单栏
    │   ├── FilePanel.h/cpp            # 文件面板 (FTP/本地/机床)
    │   ├── ProgressBar.h/cpp          # 进度条
    │   └── MachineConfigDialog.h/cpp  # 机床配置对话框
    └── core/
        ├── FileTransfer.h/cpp         # 传输管理器
        ├── FtpClient.h/cpp            # FTP客户端
        └── LocalFileOps.h/cpp         # 本地文件操作
```

## 核心组件

### MainWindow
- 三列QSplitter布局
- 菜单栏: 文件传输(刷新/退出) + 机床配置
- 协调FilePanel之间的传输

### FilePanel
- 可复用组件,支持三种类型: FTP/本地/机床
- QListWidget显示文件列表
- 路径栏 + 刷新按钮 + 服务器选择下拉框
- **异步连接**: 使用QtConcurrent在后台线程测试连接,避免UI卡顿
- **Loading状态**: 连接时显示"连接中...",禁用按钮/下拉框/文件列表
- **多选支持**: ExtendedSelection模式，支持Ctrl/Cmd多选文件拖拽

### MachineConfigDialog
- 显示机床服务器列表
- 支持添加/编辑/删除服务器
- 保存配置到 `machine_servers.ini`

### FileTransfer
- 协调4种传输路径
- 调用FtpClient/LocalFileOps
- 发送进度信号
- **批量传输**: 支持多文件批量传输，可取消

### FtpClient
- QTcpSocket实现FTP协议
- 被动模式,支持LIST/RETR/STOR

### LocalFileOps
- 静态工具类
- 列表/复制/删除/创建目录/重命名

## 配置文件格式

### ftp_servers.ini / machine_servers.ini
```ini
[Servers]
count=1

[Server_0]
name=服务器名称
host=192.168.1.100
port=21
user=username
password=password
```

---

## UI设计规范

### 视觉风格
采用**深蓝专业风** + **线性图标**，参考VS Code / 企业软件风格

### 配色方案
| 用途 | 颜色值 |
|------|--------|
| 主色调 | #2D5A8A |
| 辅助色 | #4A90D9 |
| 悬停色 | #5BA0E9 |
| 背景色 | #F5F5F5 |
| 面板背景 | #FFFFFF |
| 边框色 | #E0E0E0 |
| 成功色 | #4CAF50 |
| 警告色 | #FF9800 |

### 资源文件
```
resources/
├── resources.qrc           # Qt资源文件
├── styles/
│   └── main.qss            # 主样式表
└── icons/                  # SVG线性图标
    ├── folder.svg, file.svg, refresh.svg, up.svg
    ├── connect.svg, disconnect.svg
    ├── add.svg, edit.svg, delete.svg, save.svg, cancel.svg
    └── server.svg, machine.svg, local.svg
```

### UI组件
```
src/ui/
├── StyleManager.h/cpp      # 样式管理器
├── MainWindow.h/cpp        # 主窗口
├── FilePanel.h/cpp         # 文件面板
├── ProgressBar.h/cpp       # 进度条
└── MachineConfigDialog.h/cpp # 机床配置对话框
```

---

**项目状态**: UI美化已完成, 异步连接已实现, 批量文件传输已实现
