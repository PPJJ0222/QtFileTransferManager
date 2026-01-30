# Qt文件传输工具 - 项目架构

## 项目定位

**小型实用工具** - 专注核心功能,保持简洁

**技术栈**: Qt 6 + WebEngine, Vue 3 + TypeScript + Vite + Tailwind CSS, C++23, CMake
**目标平台**: Windows
**架构**: Web混合架构 (Qt WebEngine + Vue.js 前端)

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
├── src/
│   ├── main.cpp
│   ├── web/
│   │   ├── WebWindow.h/cpp  # WebEngine主窗口
│   │   ├── Bridge.h/cpp     # C++/JS通信桥梁
│   │   └── FtpWorker.h/cpp  # FTP工作线程
│   ├── core/
│   │   ├── FileTransfer.h/cpp   # 传输管理器
│   │   ├── FtpClient.h/cpp      # FTP客户端
│   │   └── LocalFileOps.h/cpp   # 本地文件操作
│   └── utils/
│       └── FileInfo.h           # 文件信息结构
├── web/                     # Vue.js 前端项目
│   ├── package.json
│   ├── vite.config.ts
│   ├── tsconfig.json
│   ├── index.html
│   └── src/
│       ├── main.ts
│       ├── App.vue
│       ├── bridge.ts        # Qt Bridge 封装
│       ├── assets/styles/   # 样式文件
│       │   └── main.css     # Tailwind CSS 主样式
│       ├── types/
│       │   ├── index.ts     # 主要类型定义
│       │   └── toast.ts     # Toast 通知类型
│       ├── composables/
│       │   └── useToast.ts  # Toast 通知 Composable
│       └── components/
│           ├── FilePanel.vue           # 文件浏览面板
│           ├── ProgressBar.vue         # 传输进度条
│           ├── MachineServerConfig.vue # 机床服务器配置
│           ├── base/                   # 玻璃态UI组件库
│           │   ├── GlassPanel.vue
│           │   ├── GlassButton.vue
│           │   ├── GlassInput.vue
│           │   ├── GlassDropdown.vue
│           │   ├── GlassModal.vue
│           │   ├── GlassContextMenu.vue
│           │   ├── GlassToast.vue
│           │   └── GlassToastContainer.vue
│           ├── file/                   # 文件列表组件
│           │   ├── FileListItem.vue
│           │   └── FolderListItem.vue
│           └── layout/                 # 布局组件
│               └── Header.vue
└── resources/
    ├── resources.qrc
    └── web/                 # 构建后的前端资源
```

## 核心组件

### WebWindow (Qt层)
- 基于 QWebEngineView 的主窗口
- 通过 QWebChannel 与前端通信
- 支持开发模式 (加载本地开发服务器)

### Bridge (Qt层)
- C++ 与 JavaScript 的通信桥梁
- 暴露所有核心功能给前端调用
- 通过信号向前端发送事件通知

### 前端 (Vue.js)
- Vue 3 + TypeScript + Vite
- 三列文件浏览器布局
- 响应式UI，支持拖拽传输

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

### FtpWorker (Qt层)
- FTP 工作线程
- 处理 FTP 和机床的异步连接操作
- 避免阻塞主线程

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
采用**现代Web风格**，基于 Vue.js 组件化开发

### 前端技术栈
- **框架**: Vue 3 (Composition API)
- **语言**: TypeScript 5.3
- **构建工具**: Vite 5.0
- **样式**: Tailwind CSS 4.1
- **图标**: Lucide Vue Next
- **字体**: Inter (@fontsource/inter)

### 前端组件
```
web/src/
├── App.vue              # 主应用组件
├── bridge.ts            # Qt Bridge 封装
├── types/
│   ├── index.ts         # 主要类型定义
│   └── toast.ts         # Toast 通知类型
├── composables/
│   └── useToast.ts      # Toast 通知 Composable
└── components/
    ├── FilePanel.vue           # 文件浏览面板
    ├── ProgressBar.vue         # 传输进度条
    ├── MachineServerConfig.vue # 机床服务器配置弹窗
    ├── base/                   # 玻璃态UI组件库
    │   ├── GlassPanel.vue      # 玻璃态面板
    │   ├── GlassButton.vue     # 玻璃态按钮
    │   ├── GlassInput.vue      # 玻璃态输入框
    │   ├── GlassDropdown.vue   # 玻璃态下拉菜单
    │   ├── GlassModal.vue      # 玻璃态模态框
    │   ├── GlassContextMenu.vue # 玻璃态右键菜单
    │   ├── GlassToast.vue      # 玻璃态通知
    │   └── GlassToastContainer.vue # Toast容器
    ├── file/                   # 文件列表组件
    │   ├── FileListItem.vue    # 文件列表项
    │   └── FolderListItem.vue  # 文件夹列表项
    └── layout/                 # 布局组件
        └── Header.vue          # 顶部菜单栏
```

### 开发模式
- 开发时: 前端运行 `npm run dev`，Qt 加载 localhost
- 生产时: 前端构建到 `resources/web/`，Qt 加载本地资源

### 玻璃态UI组件库 (base/)
项目采用统一的玻璃态(Glassmorphism)设计风格：
- **GlassPanel** - 基础容器面板
- **GlassButton** - 按钮组件
- **GlassInput** - 输入框组件
- **GlassDropdown** - 下拉选择器
- **GlassModal** - 模态对话框
- **GlassContextMenu** - 右键上下文菜单
- **GlassToast** - 通知提示组件

---

**项目状态**: Web 混合架构开发中，已完成玻璃态UI组件库和核心传输功能
