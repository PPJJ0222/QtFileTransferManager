<script setup lang="ts">
import { ref, reactive, provide, onMounted } from 'vue'
import FilePanel from './components/FilePanel.vue'
import ProgressBar from './components/ProgressBar.vue'
import { Header } from './components/layout'
import { GlassToastContainer } from './components/base'
import { getBridge, onBridgeReady } from './bridge'
import { useToast } from './composables/useToast'
import type { PanelType, PanelState, TransferTask, TransferContext, TransferItem } from './types'

const toast = useToast()

// 面板状态管理
const panelStates = reactive<Record<PanelType, PanelState>>({
  ftp: { currentPath: '/', isConnected: false },
  local: { currentPath: '/', isConnected: true },
  machine: { currentPath: '/', isConnected: false }
})

// 传输状态
const progress = ref(0)
const progressText = ref('')
const isTransferring = ref(false)
const currentTask = ref<TransferTask | null>(null)

// 批量传输进度
const currentIndex = ref(0)
const totalCount = ref(0)

// 更新面板状态（由子组件调用）
function updatePanelState(type: PanelType, state: Partial<PanelState>) {
  Object.assign(panelStates[type], state)
}

// 面板刷新回调注册
const panelRefreshCallbacks = new Map<PanelType, () => void>()

function registerPanelRefresh(type: PanelType, callback: () => void) {
  panelRefreshCallbacks.set(type, callback)
}

function refreshPanel(type: PanelType) {
  const callback = panelRefreshCallbacks.get(type)
  if (callback) callback()
}

// 传输下一个文件
function transferNextFile() {
  const task = currentTask.value
  if (!task || task.status === 'cancelled') return

  const bridge = getBridge()
  if (!bridge) return

  // 更新批量进度
  totalCount.value = task.sourceItems.length
  currentIndex.value = task.currentIndex + 1

  if (task.currentIndex >= task.sourceItems.length) {
    // 所有文件传输完成
    task.status = 'completed'
    isTransferring.value = false
    progressText.value = '传输完成'
    return
  }

  const item = task.sourceItems[task.currentIndex]
  const fullSourcePath = `${task.sourcePath}/${item.name}`.replace(/\/+/g, '/')
  const fullDestPath = `${task.destPath}/${item.name}`.replace(/\/+/g, '/')

  progressText.value = item.name

  // 根据是文件还是文件夹调用不同的传输方法
  if (item.isDir) {
    bridge.transferDirectory(fullSourcePath, fullDestPath, task.sourceType, task.destType)
  } else {
    bridge.transferFile(fullSourcePath, fullDestPath, task.sourceType, task.destType)
  }
}

// 开始传输
function startTransfer(sourceType: PanelType, destType: PanelType, items: TransferItem[]) {
  const bridge = getBridge()
  if (!bridge) return

  const sourcePath = panelStates[sourceType].currentPath
  const destPath = panelStates[destType].currentPath

  // 创建传输任务
  currentTask.value = {
    sourceItems: items,
    sourcePath,
    destPath,
    sourceType,
    destType,
    currentIndex: 0,
    status: 'transferring'
  }

  isTransferring.value = true
  progress.value = 0

  // 开始传输第一个文件
  transferNextFile()
}

// 取消传输
function cancelTransfer() {
  const bridge = getBridge()
  if (bridge) {
    bridge.cancelTransfer()
  }
  if (currentTask.value) {
    currentTask.value.status = 'cancelled'
  }
  isTransferring.value = false
  progressText.value = '已取消'
}

// 监听 Bridge 信号
onMounted(() => {
  onBridgeReady(() => {
    const bridge = getBridge() as any
    if (!bridge) return

    // 监听传输进度
    if (bridge.transferProgress?.connect) {
      bridge.transferProgress.connect((_current: number, _total: number, percent: number) => {
        progress.value = percent
      })
    }

    // 监听传输完成
    if (bridge.transferCompleted?.connect) {
      bridge.transferCompleted.connect((success: boolean, message: string) => {
        if (!currentTask.value) return

        if (success) {
          currentTask.value.currentIndex++

          // 检查是否全部完成
          if (currentTask.value.currentIndex >= currentTask.value.sourceItems.length) {
            // 显示成功提示
            const count = currentTask.value.sourceItems.length
            toast.success(`成功传输 ${count} 个项目`)

            // 刷新源和目标面板
            refreshPanel(currentTask.value.sourceType)
            refreshPanel(currentTask.value.destType)
          }

          transferNextFile()
        } else {
          currentTask.value.status = 'error'
          isTransferring.value = false
          toast.error(message || '传输失败')
        }
      })
    }

    // 监听传输错误
    if (bridge.transferError?.connect) {
      bridge.transferError.connect((error: string) => {
        if (currentTask.value) {
          currentTask.value.status = 'error'
        }
        isTransferring.value = false
        toast.error(error || '传输错误')
      })
    }
  })
})

// 提供给子组件
provide<TransferContext>('transferContext', {
  panelStates,
  isTransferring,
  startTransfer,
  cancelTransfer,
  updatePanelState,
  refreshPanel,
  registerPanelRefresh
})

function handleRefresh() {
  // 刷新所有面板
}

function handleSettings() {
  // 打开设置
}
</script>

<template>
  <div class="main-screen select-none" @contextmenu.prevent>
    <!-- 背景装饰层 -->
    <div class="background-decorations">
      <div class="glow glow-blue"></div>
      <div class="glow glow-purple"></div>
      <div class="glow glow-pink"></div>
      <div class="glow glow-cyan"></div>
      <div class="glow glow-green"></div>
    </div>

    <!-- 内容层 -->
    <div class="content">
      <!-- 头部 -->
      <Header
        title="文件传输管理器"
        @refresh="handleRefresh"
        @settings="handleSettings"
      />

      <!-- 主内容区 -->
      <main class="panels-row">
        <FilePanel type="ftp" title="FTP 服务器" :isTransferring="isTransferring" />
        <FilePanel type="local" title="本地文件" :isTransferring="isTransferring" />
        <FilePanel type="machine" title="机床" :isTransferring="isTransferring" />
      </main>

      <!-- 底部进度条 -->
      <ProgressBar
        :progress="progress"
        :text="progressText"
        :visible="isTransferring"
        :currentIndex="currentIndex"
        :totalCount="totalCount"
        @cancel="cancelTransfer"
      />
    </div>

    <!-- Toast 容器 -->
    <GlassToastContainer />
  </div>
</template>

<style scoped>
.main-screen {
  position: relative;
  width: 100%;
  height: 100vh;
  background: var(--color-bg-page);
  overflow: hidden;
}

.background-decorations {
  position: absolute;
  inset: 0;
  pointer-events: none;
  overflow: hidden;
}

.glow {
  position: absolute;
  border-radius: 50%;
  filter: blur(60px);
}

.glow-blue {
  width: 400px;
  height: 400px;
  left: -100px;
  top: -100px;
  background: radial-gradient(circle, #60A5FA40 0%, #60A5FA00 100%);
}

.glow-purple {
  width: 350px;
  height: 350px;
  right: -50px;
  top: -50px;
  background: radial-gradient(circle, #A78BFA30 0%, #A78BFA00 100%);
}

.glow-pink {
  width: 300px;
  height: 300px;
  left: 33%;
  bottom: 50px;
  background: radial-gradient(circle, #F472B620 0%, #F472B600 100%);
}

.glow-cyan {
  width: 350px;
  height: 350px;
  right: 100px;
  bottom: -50px;
  background: radial-gradient(circle, #22D3EE20 0%, #22D3EE00 100%);
}

.glow-green {
  width: 280px;
  height: 280px;
  left: -50px;
  bottom: 100px;
  background: radial-gradient(circle, #4ADE8020 0%, #4ADE8000 100%);
}

.content {
  position: relative;
  z-index: 1;
  display: flex;
  flex-direction: column;
  gap: 16px;
  padding: 20px;
  height: 100%;
}

.panels-row {
  flex: 1;
  display: flex;
  gap: 20px;
  min-height: 0;
}
</style>
