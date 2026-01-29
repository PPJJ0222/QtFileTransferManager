import { ref, readonly } from 'vue'
import type { ToastOptions, ToastInstance } from '../types/toast'

// 全局状态（单例模式）
const toasts = ref<ToastInstance[]>([])
let toastId = 0

// 生成唯一ID
function generateId(): string {
  return `toast-${++toastId}-${Date.now()}`
}

// 显示Toast
function showToast(options: ToastOptions): string {
  const id = generateId()

  const instance: ToastInstance = {
    id,
    type: options.type,
    message: options.message,
    duration: options.duration ?? 3000
  }

  toasts.value.push(instance)

  // 自动关闭
  if (instance.duration > 0) {
    setTimeout(() => {
      removeToast(id)
    }, instance.duration)
  }

  return id
}

// 移除Toast
function removeToast(id: string): void {
  const index = toasts.value.findIndex(t => t.id === id)
  if (index > -1) {
    toasts.value.splice(index, 1)
  }
}

// 清空所有Toast
function clearAll(): void {
  toasts.value = []
}

// 快捷方法
function success(message: string, duration?: number): string {
  return showToast({ type: 'success', message, duration })
}

function warning(message: string, duration?: number): string {
  return showToast({ type: 'warning', message, duration })
}

function error(message: string, duration?: number): string {
  return showToast({ type: 'error', message, duration })
}

function info(message: string, duration?: number): string {
  return showToast({ type: 'info', message, duration })
}

// 导出 Composable
export function useToast() {
  return {
    toasts: readonly(toasts),
    showToast,
    removeToast,
    clearAll,
    success,
    warning,
    error,
    info
  }
}

// 导出内部 toasts（用于容器组件）
export { toasts }
