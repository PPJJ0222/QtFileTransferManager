import type { QtBridge } from './types'

let bridge: QtBridge | null = null
let bridgeReady = false
const pendingCallbacks: Array<() => void> = []

// 初始化 Qt WebChannel 连接
export function initBridge(): Promise<QtBridge> {
  return new Promise((resolve, reject) => {
    if (bridge && bridgeReady) {
      resolve(bridge)
      return
    }

    // 检查是否在 Qt 环境中
    if (typeof (window as any).qt !== 'undefined') {
      new (window as any).QWebChannel(
        (window as any).qt.webChannelTransport,
        (channel: any) => {
          bridge = channel.objects.bridge as QtBridge
          bridgeReady = true
          pendingCallbacks.forEach(cb => cb())
          pendingCallbacks.length = 0
          resolve(bridge)
        }
      )
    } else {
      reject(new Error('Qt 环境未检测到，无法初始化 Bridge'))
    }
  })
}

export function getBridge(): QtBridge | null {
  return bridge
}

export function onBridgeReady(callback: () => void): void {
  if (bridgeReady) {
    callback()
  } else {
    pendingCallbacks.push(callback)
  }
}
