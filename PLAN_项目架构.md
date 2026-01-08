# Qt文件传输工具 - 优化实施计划

## 项目定位

**小型实用工具** - 专注核心功能,保持简洁

**技术栈**: Qt 6.10.1, C++23, CMake
**目标平台**: Windows (暂不考虑macOS)
**架构**: 简化的UI+Core两层架构

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
- 三列文件浏览器 (FTP | 本地 | 机床)
- 支持拖拽传输
- 简单进度显示

## 简化的项目结构

```
QtFileTransferManager/
├── CMakeLists.txt
├── README.md
├── ftp_servers.ini          # FTP服务器配置文件
└── src/
    ├── main.cpp
    ├── ui/                      # UI层
    │   ├── MainWindow.h/cpp     # 三列布局主窗口
    │   ├── FilePanel.h/cpp      # 单个文件面板(可复用)
    │   └── ProgressBar.h/cpp    # 简单进度条
    ├── core/                    # 核心业务层
    │   ├── FileTransfer.h/cpp   # 传输管理器
    │   ├── FtpClient.h/cpp      # FTP客户端
    │   ├── MachineClient.h/cpp  # 机床客户端抽象接口
    │   └── LocalFileOps.h/cpp   # 本地文件操作
    └── utils/
        └── FileInfo.h           # 文件信息结构体
```

**架构简化**:
- 去除MVC的Controller层,UI直接调用core层
- 去除独立的Model层,使用简单数据结构
- 去除Delegate,使用QListWidget默认渲染
- 最小化依赖: 只用Qt Core/Widgets/Network

## 核心组件设计

### 1. MainWindow (主窗口)
- 三列QSplitter布局
- 协调FilePanel之间的传输
- 处理拖拽事件并调用FileTransfer

**关键文件**: `src/ui/MainWindow.h/cpp`

### 2. FilePanel (文件面板)
- 可复用组件,支持三种类型: FTP/本地/机床
- QListWidget显示文件列表
- 路径栏 + 刷新按钮
- 拖拽支持
- FTP面板: 下拉框选择服务器 + 连接按钮（配置从ftp_servers.ini读取）

**关键文件**: `src/ui/FilePanel.h/cpp`

### 3. FileTransfer (传输管理器)
- 协调4种传输路径
- 调用FtpClient/MachineClient/LocalFileOps
- 发送进度信号

**关键文件**: `src/core/FileTransfer.h/cpp`

### 4. MachineClient (机床客户端抽象接口)
- 抽象基类,定义统一接口
- 支持多种协议实现 (TCP/串口/自定义)
- 异步操作,使用Qt信号槽

**接口方法**:
```cpp
virtual bool connect(QString host, int port) = 0;
virtual QStringList listFiles(QString path) = 0;
virtual bool downloadFile(QString remotePath, QString localPath) = 0;
virtual bool uploadFile(QString localPath, QString remotePath) = 0;
virtual bool deleteFile(QString remotePath) = 0;
```

**扩展性**: 可实现MachineTcpClient、MachineSerialClient等具体版本

**关键文件**: `src/core/MachineClient.h/cpp`

### 5. FtpClient (FTP客户端)
- 使用QTcpSocket实现FTP协议
- 只支持被动模式,简化实现
- 支持基础命令: LIST, RETR, STOR

**关键文件**: `src/core/FtpClient.h/cpp`

### 6. LocalFileOps (本地文件操作)
- 静态工具类
- 封装本地文件操作: 列表/复制/删除/创建目录/重命名

**关键文件**: `src/core/LocalFileOps.h/cpp`

## 实施步骤 (28步)

### 阶段1: 基础框架 (5步) ✅ 已完成
1. ✅ 创建项目目录结构
2. ✅ 配置CMakeLists.txt (Windows路径: `C:/Qt/6.10.1/msvc2022_64`)
3. ✅ 实现main.cpp和基础QApplication
4. ✅ 实现MainWindow三列QSplitter布局
5. ✅ 实现FilePanel基础UI (QListWidget + 路径栏 + 刷新按钮)

### 阶段2: 本地文件系统 (4步) ✅ 已完成
6. ✅ 实现LocalFileOps类 (列表/复制/删除/创建/重命名)
7. ✅ 集成到中间FilePanel,显示本地文件列表
8. ✅ 添加本地文件操作功能 (右键菜单: 删除/重命名/新建文件夹)
9. ✅ 测试本地文件浏览和操作

### 阶段3: FTP客户端 (5步) ✅ 已完成
10. ✅ 实现FtpClient基础连接和认证 (USER/PASS命令)
11. ✅ 实现FTP LIST命令和响应解析
12. ✅ 实现FTP下载 (RETR命令)
13. ✅ 实现FTP上传 (STOR命令)
14. ✅ 集成到左侧FilePanel并测试FTP浏览

### 阶段4: 机床客户端接口 (5步) ✅ 已完成
15. ✅ 定义MachineClient抽象接口
16. ✅ 实现MachineTcpClient骨架 (使用简单TLV协议)
17. ✅ 实现机床文件列表获取
18. ✅ 实现机床文件下载
19. ✅ 实现机床文件上传

### 阶段5: 传输系统 (5步) ✅ 已完成
20. ✅ 实现FileTransfer类框架
21. ✅ 实现FTP→本地和本地→FTP传输
22. ✅ 实现机床→本地和本地→机床传输
23. ✅ 添加ProgressBar显示传输进度
24. ✅ 测试所有4种传输路径

### 阶段6: 拖拽和集成 (4步) ✅ 已完成
25. ✅ 为FilePanel添加拖拽支持 (dragEnterEvent/dropEvent)
26. ✅ 连接拖拽事件到FileTransfer
27. ✅ 添加错误处理和状态提示
28. ✅ 完整测试和优化

## 关键技术决策

### Windows特定配置
- CMake路径: `C:/Qt/6.10.1/msvc2022_64`
- 使用`QDir::toNativeSeparators()`确保反斜杠路径
- 使用`QFileIconProvider`获取Windows系统图标

### 简化设计原则
- **不使用QFileSystemModel**: 直接用QListWidget,减少抽象
- **不实现传输队列**: 单次传输,简化逻辑
- **不支持暂停/恢复**: 传输失败重新开始
- **同步UI更新**: 使用QProgressDialog阻塞,简化线程管理

### 机床协议扩展性
- 使用工厂模式创建不同协议客户端
- 支持TCP/串口/自定义协议
- 建议使用简单的TLV格式 (Type-Length-Value) 如果是自定义协议

## CMakeLists.txt配置要点

```cmake
cmake_minimum_required(VERSION 3.16)
project(QtFileTransferManager)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.10.1/msvc2022_64")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

add_executable(QtFileTransferManager
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/FilePanel.cpp
    src/ui/ProgressBar.cpp
    src/core/FileTransfer.cpp
    src/core/FtpClient.cpp
    src/core/MachineClient.cpp
    src/core/LocalFileOps.cpp
)

target_link_libraries(QtFileTransferManager
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)
```

## 与原计划对比

| 项目 | 原计划 | 优化方案 |
|------|--------|----------|
| 实施步骤 | 57步 | 28步 |
| 传输路径 | 6种(包括FTP↔机床) | 4种(本地中转) |
| 架构层次 | MVC+Service(4层) | UI+Core(2层) |
| 文件数量 | ~20个类 | ~10个类 |
| 目标平台 | macOS跨平台 | Windows专用 |
| UI复杂度 | QTreeView+Delegate | QListWidget默认 |

## 后续可选增强 (MVP后)

- 批量文件传输
- 传输历史记录
- 文件搜索功能
- 更丰富的错误提示

## FTP服务器配置

配置文件 `ftp_servers.ini` 与可执行文件放在同一目录，格式如下：

```ini
[Servers]
count=2

[Server_0]
name=大机程序服务器
host=192.168.18.9
port=21
user=SJK
password=sam@123

[Server_1]
name=小机手动程序服务器
host=192.168.6.19
port=21
user=xjjk
password=sam123
```

添加新服务器：增加count值，添加对应的Server_N节。

---

**当前进度**: 所有阶段已完成 (28/28步) ✅

**项目状态**: MVP开发完成
