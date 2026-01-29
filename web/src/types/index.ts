import type { Ref } from 'vue'

// 面板类型
export type PanelType = 'ftp' | 'local' | 'machine'

// 文件信息类型
export interface FileInfo {
  name: string
  isDir: boolean
  size: number
  modifiedTime: string
}

// 服务器配置类型
export interface ServerConfig {
  name: string
  host: string
  port: number
  user: string
  password: string
}

// 传输进度类型
export interface TransferProgress {
  current: number
  total: number
  percentage: number
  fileName: string
}

// Bridge 接口类型
export interface QtBridge {
  // FTP 操作
  connectFtp(host: string, port: number, user: string, password: string): void
  disconnectFtp(): void
  listFtpDirectory(path: string): void

  // 本地文件操作
  listLocalDirectory(path: string): void
  deleteLocalFiles(basePath: string, fileNames: string[]): void

  // 机床操作
  connectMachine(host: string, port: number, user: string, password: string): void
  disconnectMachine(): void
  listMachineDirectory(path: string): void

  // 文件传输
  transferFile(sourcePath: string, destPath: string, sourceType: string, destType: string): void
  transferDirectory(sourcePath: string, destPath: string, sourceType: string, destType: string): void
  cancelTransfer(): void

  // 配置管理
  loadFtpServers(): void
  loadMachineServers(): void
  saveMachineServer(config: ServerConfig): void
  deleteMachineServer(name: string): void
}

// 面板状态（用于跨组件共享）
export interface PanelState {
  currentPath: string
  isConnected: boolean
}

// 传输项（文件或文件夹）
export interface TransferItem {
  name: string
  isDir: boolean
}

// 传输任务
export interface TransferTask {
  sourceItems: TransferItem[]
  sourcePath: string
  destPath: string
  sourceType: PanelType
  destType: PanelType
  currentIndex: number
  status: 'transferring' | 'completed' | 'error' | 'cancelled'
}

// 传输上下文（provide/inject）
export interface TransferContext {
  panelStates: Record<PanelType, PanelState>
  isTransferring: Ref<boolean>
  startTransfer: (sourceType: PanelType, destType: PanelType, items: TransferItem[]) => void
  cancelTransfer: () => void
  updatePanelState: (type: PanelType, state: Partial<PanelState>) => void
  refreshPanel: (type: PanelType) => void
  registerPanelRefresh: (type: PanelType, callback: () => void) => void
}
