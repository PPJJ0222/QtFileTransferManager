// Toast 类型
export type ToastType = 'success' | 'warning' | 'error' | 'info'

// Toast 配置项
export interface ToastOptions {
  type: ToastType
  message: string
  duration?: number  // 显示时长（毫秒），默认3000，0表示不自动关闭
}

// Toast 实例（内部使用）
export interface ToastInstance {
  id: string
  type: ToastType
  message: string
  duration: number  // 必需，已有默认值
}
