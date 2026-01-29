<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted, watch, inject } from 'vue'
import { Server, HardDrive, Monitor, FolderOpen } from 'lucide-vue-next'
import type { FileInfo, ServerConfig, TransferContext, PanelType, TransferItem } from '../types'
import { getBridge } from '../bridge'
import { GlassPanel, GlassButton, GlassInput, GlassDropdown, GlassModal, GlassContextMenu } from './base'
import type { ContextMenuItem } from './base'
import { FileListItem, FolderListItem } from './file'
import { useToast } from '../composables/useToast'

const toast = useToast()

const props = defineProps<{
  type: 'ftp' | 'local' | 'machine'
  title: string
  isTransferring?: boolean
}>()

// 注入传输上下文
const transferContext = inject<TransferContext>('transferContext')!

const currentPath = ref(props.type === 'local' ? '/' : '/')
const files = ref<FileInfo[]>([])
const selectedFiles = ref<string[]>([])
const lastSelectedIndex = ref<number>(-1)
const isConnected = ref(props.type === 'local')
const isLoading = ref(false)
const servers = ref<ServerConfig[]>([])
const selectedServer = ref('')

// 切换服务器确认相关状态
const showSwitchConfirm = ref(false)
const pendingServer = ref('')

// 删除确认相关状态
const showDeleteConfirm = ref(false)
const filesToDelete = ref<string[]>([])

// 框选相关状态
const fileListRef = ref<HTMLElement | null>(null)
const isSelecting = ref(false)
const selectionBox = ref({ startX: 0, startY: 0, x: 0, y: 0, width: 0, height: 0 })

// 右键菜单状态
const showContextMenu = ref(false)
const contextMenuX = ref(0)
const contextMenuY = ref(0)

// 面板聚焦状态
const isFocused = ref(false)

// 拖放相关状态
const isDragOver = ref(false)

const showServerSelect = computed(() => props.type !== 'local')

const serverOptions = computed(() =>
  servers.value.map(s => ({ label: s.name, value: s.name }))
)

const panelIcon = computed(() => {
  const icons = { ftp: Server, local: HardDrive, machine: Monitor }
  return icons[props.type]
})

// 右键菜单项
const contextMenuItems = computed<ContextMenuItem[]>(() => {
  if (props.type === 'local') {
    return [
      { label: '传输到程序服务器', action: 'transfer-to-ftp' },
      { label: '传输到机床', action: 'transfer-to-machine' },
      { label: '删除', action: 'delete' }
    ]
  } else {
    return [
      { label: '传输到本地', action: 'transfer-to-local' }
    ]
  }
})

// 安全连接信号的辅助函数
function connectSignal(signal: any, callback: (...args: any[]) => void) {
  if (signal && typeof signal.connect === 'function') {
    signal.connect(callback)
  }
}

// 初始化信号监听
onMounted(() => {
  // 添加键盘事件监听
  document.addEventListener('keydown', handleKeyDown)

  // 注册面板刷新回调
  transferContext.registerPanelRefresh(props.type, loadDirectory)

  const bridge = getBridge()
  if (!bridge) return

  const b = bridge as any

  if (props.type === 'local') {
    connectSignal(b.localDirectoryListed, (list: any[]) => {
      files.value = list.map(f => ({
        name: f.name,
        isDir: f.isDir,
        size: f.size || 0,
        modifiedTime: ''
      }))
      isLoading.value = false
    })
    // 监听删除完成信号
    connectSignal(b.localFilesDeleted, (success: boolean, message: string) => {
      if (success) {
        selectedFiles.value = []
        loadDirectory()
        toast.success('删除成功')
      } else {
        toast.error('删除失败: ' + message)
      }
    })
    loadDirectory()
  } else if (props.type === 'ftp') {
    // 监听 FTP 服务器列表加载完成信号
    connectSignal(b.ftpServersLoaded, (list: any[]) => {
      servers.value = list.map(s => ({
        name: s.name,
        host: s.host,
        port: s.port,
        user: s.user,
        password: s.password
      }))
    })
    // 监听 FTP 目录列表信号
    connectSignal(b.ftpDirectoryListed, (list: any[]) => {
      files.value = list.map(f => ({
        name: f.name,
        isDir: f.isDir,
        size: f.size || 0,
        modifiedTime: ''
      }))
      isLoading.value = false
    })
    // 监听 FTP 连接成功信号
    connectSignal(b.ftpConnected, () => {
      isConnected.value = true
      isLoading.value = false
      loadDirectory()
    })
    // 加载 FTP 服务器列表
    bridge.loadFtpServers()
  } else if (props.type === 'machine') {
    // 监听机床服务器列表加载完成信号
    connectSignal(b.machineServersLoaded, (list: any[]) => {
      servers.value = list.map(s => ({
        name: s.name,
        host: s.host,
        port: s.port,
        user: s.user,
        password: s.password
      }))
    })
    // 监听机床目录列表信号
    connectSignal(b.machineDirectoryListed, (list: any[]) => {
      files.value = list.map(f => ({
        name: f.name,
        isDir: f.isDir,
        size: f.size || 0,
        modifiedTime: ''
      }))
      isLoading.value = false
    })
    // 监听机床连接成功信号
    connectSignal(b.machineConnected, () => {
      isConnected.value = true
      isLoading.value = false
      loadDirectory()
    })
    // 加载机床服务器列表
    bridge.loadMachineServers()
  }
})

function loadDirectory() {
  const bridge = getBridge()
  if (!bridge) return

  // FTP/机床面板未连接时不执行刷新
  if (props.type !== 'local' && !isConnected.value) {
    return
  }

  isLoading.value = true
  if (props.type === 'local') {
    bridge.listLocalDirectory(currentPath.value)
  } else if (props.type === 'ftp') {
    bridge.listFtpDirectory(currentPath.value)
  } else if (props.type === 'machine') {
    bridge.listMachineDirectory(currentPath.value)
  }
}

// 处理服务器选择变化
function handleServerChange(newServer: string | number) {
  const serverName = String(newServer)

  // 如果未连接，直接更新选择
  if (!isConnected.value) {
    selectedServer.value = serverName
    return
  }

  // 如果正在传输，弹出确认对话框
  if (props.isTransferring) {
    pendingServer.value = serverName
    showSwitchConfirm.value = true
    return
  }

  // 未在传输，直接切换
  doSwitchServer(serverName)
}

// 执行服务器切换
function doSwitchServer(newServer: string) {
  const bridge = getBridge()
  if (!bridge) return

  // 断开当前连接
  if (props.type === 'ftp') {
    bridge.disconnectFtp()
  } else if (props.type === 'machine') {
    bridge.disconnectMachine()
  }

  // 重置状态
  isConnected.value = false
  files.value = []
  currentPath.value = '/'

  // 更新选择
  selectedServer.value = newServer
}

// 确认切换服务器
function confirmSwitchServer() {
  const bridge = getBridge()
  if (bridge) {
    // 取消当前传输
    bridge.cancelTransfer()
  }

  doSwitchServer(pendingServer.value)
  showSwitchConfirm.value = false
  pendingServer.value = ''
}

// 取消切换服务器
function cancelSwitchServer() {
  showSwitchConfirm.value = false
  pendingServer.value = ''
}

// 右键菜单处理
function handleContextMenu(event: MouseEvent) {
  // 阻止默认右键菜单
  event.preventDefault()

  // 检查是否点击在文件项上
  const target = event.target as HTMLElement
  const fileItem = target.closest('[data-file-item]')

  // 如果不是点击在文件项上，清空选中并不显示菜单
  if (!fileItem) {
    selectedFiles.value = []
    return
  }

  const fileName = fileItem.getAttribute('data-file-name')
  if (fileName) {
    // 如果点击的文件未被选中，则选中它
    if (!selectedFiles.value.includes(fileName)) {
      selectedFiles.value = [fileName]
    }
  }

  // 如果没有选中文件，不显示菜单
  if (selectedFiles.value.length === 0) return

  // 显示自定义右键菜单
  contextMenuX.value = event.clientX
  contextMenuY.value = event.clientY
  showContextMenu.value = true
}

function handleContextMenuSelect(action: string) {
  showContextMenu.value = false

  // 处理删除操作
  if (action === 'delete') {
    filesToDelete.value = [...selectedFiles.value]
    showDeleteConfirm.value = true
    return
  }

  // 确定目标面板类型
  let destType: PanelType
  switch (action) {
    case 'transfer-to-ftp':
      destType = 'ftp'
      break
    case 'transfer-to-local':
      destType = 'local'
      break
    case 'transfer-to-machine':
      destType = 'machine'
      break
    default:
      return
  }

  // 检查目标面板是否已连接
  if (!transferContext.panelStates[destType].isConnected) {
    toast.warning('目标面板未连接，请先连接服务器')
    return
  }

  // 构建传输项列表（包含文件和文件夹）
  const itemsToTransfer: TransferItem[] = selectedFiles.value
    .map(name => {
      const file = files.value.find(f => f.name === name)
      return file ? { name: file.name, isDir: file.isDir } : null
    })
    .filter((item): item is TransferItem => item !== null)

  if (itemsToTransfer.length === 0) {
    toast.warning('没有可传输的项目')
    return
  }

  // 调用传输方法
  transferContext.startTransfer(props.type, destType, itemsToTransfer)
}

function closeContextMenu() {
  showContextMenu.value = false
}

// 确认删除
function confirmDelete() {
  const bridge = getBridge()
  if (!bridge) return

  bridge.deleteLocalFiles(currentPath.value, filesToDelete.value)
  showDeleteConfirm.value = false
  filesToDelete.value = []
}

// 取消删除
function cancelDelete() {
  showDeleteConfirm.value = false
  filesToDelete.value = []
}

// 监听路径变化，上报给父组件
watch(currentPath, (newPath) => {
  transferContext.updatePanelState(props.type, { currentPath: newPath })
  if (isConnected.value) {
    loadDirectory()
  }
}, { immediate: true })

// 监听连接状态变化，上报给父组件
watch(isConnected, (connected) => {
  transferContext.updatePanelState(props.type, { isConnected: connected })
}, { immediate: true })

function handleConnect() {
  const bridge = getBridge()
  if (!bridge) return

  // 如果已连接，则断开
  if (isConnected.value) {
    if (props.type === 'ftp') {
      bridge.disconnectFtp()
    } else if (props.type === 'machine') {
      bridge.disconnectMachine()
    }
    isConnected.value = false
    files.value = []
    return
  }

  // 获取选中的服务器配置
  const server = servers.value.find(s => s.name === selectedServer.value)
  if (!server) return

  isLoading.value = true

  // 直接调用连接方法，信号已在 onMounted 中连接
  if (props.type === 'ftp') {
    bridge.connectFtp(server.host, server.port, server.user, server.password)
  } else if (props.type === 'machine') {
    bridge.connectMachine(server.host, server.port, server.user, server.password)
  }
}

function handleRefresh() {
  loadDirectory()
}

function handleGoUp() {
  const parts = currentPath.value.split('/').filter(Boolean)
  parts.pop()
  currentPath.value = '/' + parts.join('/') || '/'
}

function handleFileClick(file: FileInfo) {
  // 如果正在加载，阻止重复操作
  if (isLoading.value) return

  if (file.isDir) {
    currentPath.value = currentPath.value + '/' + file.name
  }
}

function toggleSelect(name: string, event: MouseEvent) {
  const currentIndex = files.value.findIndex(f => f.name === name)

  if (event.shiftKey && lastSelectedIndex.value !== -1) {
    // Shift+点击：范围选择
    const start = Math.min(lastSelectedIndex.value, currentIndex)
    const end = Math.max(lastSelectedIndex.value, currentIndex)
    const rangeNames = files.value.slice(start, end + 1).map(f => f.name)

    if (event.ctrlKey || event.metaKey) {
      // Shift+Ctrl：添加范围到现有选择
      const newSelection = new Set([...selectedFiles.value, ...rangeNames])
      selectedFiles.value = Array.from(newSelection)
    } else {
      // 仅Shift：替换为范围选择
      selectedFiles.value = rangeNames
    }
  } else if (event.ctrlKey || event.metaKey) {
    // Ctrl/Cmd+点击：切换单个项目
    const index = selectedFiles.value.indexOf(name)
    if (index > -1) {
      selectedFiles.value.splice(index, 1)
    } else {
      selectedFiles.value.push(name)
    }
    lastSelectedIndex.value = currentIndex
  } else {
    // 普通点击：单选
    const index = selectedFiles.value.indexOf(name)
    if (index > -1) {
      selectedFiles.value = []
    } else {
      selectedFiles.value = [name]
    }
    lastSelectedIndex.value = currentIndex
  }
}

// 全选功能 (Cmd+A / Ctrl+A)
function selectAll() {
  selectedFiles.value = files.value.map(f => f.name)
}

function handleKeyDown(event: KeyboardEvent) {
  // 只在当前面板聚焦时响应
  if (!isFocused.value) return

  // Cmd+A (macOS) 或 Ctrl+A (Windows/Linux)
  if ((event.metaKey || event.ctrlKey) && event.key === 'a') {
    event.preventDefault()
    selectAll()
  }
}

// 面板聚焦/失焦处理
function handlePanelFocus() {
  isFocused.value = true
}

function handlePanelBlur(event: FocusEvent) {
  // 检查焦点是否移出了面板（而不是移到面板内的其他元素）
  const relatedTarget = event.relatedTarget as HTMLElement | null
  if (!fileListRef.value?.contains(relatedTarget)) {
    isFocused.value = false
  }
}

// 拖动开始处理
function handleDragStart(event: DragEvent, file: FileInfo) {
  if (!event.dataTransfer) return

  // 如果拖动的文件未被选中，则只选中它
  if (!selectedFiles.value.includes(file.name)) {
    selectedFiles.value = [file.name]
  }

  const itemCount = selectedFiles.value.length

  // 创建自定义拖动图像
  if (itemCount > 1) {
    const dragImage = document.createElement('div')
    dragImage.className = 'fixed pointer-events-none bg-white/95 backdrop-blur-sm rounded-lg shadow-lg border border-[#E5E7EB] px-3 py-2 flex items-center gap-2'
    dragImage.innerHTML = `
      <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="text-[var(--color-primary)]">
        <path d="M15 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V7Z"/>
        <path d="M14 2v4a2 2 0 0 0 2 2h4"/>
      </svg>
      <span class="text-sm font-medium text-[var(--color-text-primary)]">${itemCount} 个项目</span>
    `
    dragImage.style.left = '-9999px'
    dragImage.style.top = '-9999px'
    document.body.appendChild(dragImage)
    event.dataTransfer.setDragImage(dragImage, 0, 0)
    // 延迟移除，确保拖动图像已被捕获
    setTimeout(() => document.body.removeChild(dragImage), 0)
  }

  // 构建拖动数据
  const dragData = {
    sourceType: props.type,
    sourcePath: currentPath.value,
    items: selectedFiles.value.map(name => {
      const f = files.value.find(item => item.name === name)
      return f ? { name: f.name, isDir: f.isDir } : null
    }).filter(Boolean)
  }

  event.dataTransfer.setData('application/json', JSON.stringify(dragData))
  event.dataTransfer.effectAllowed = 'copy'
}

// 拖动经过处理 - 判断是否允许放置
function handleDragOver(event: DragEvent) {
  if (!event.dataTransfer) return

  const data = event.dataTransfer.types.includes('application/json')
  if (!data) return

  // 尝试获取拖动数据来判断来源
  // 注意：dragover 事件中无法直接读取 dataTransfer 数据，需要通过其他方式判断
  // 这里我们通过 transferContext 来获取当前拖动的来源信息

  // 简单的规则：
  // - 本地面板可以接收来自 FTP 和机床的文件
  // - FTP 和机床面板可以接收来自本地的文件
  // - 不能拖放到自己

  // 由于 dragover 无法读取数据，我们先允许放置，在 drop 时再验证
  event.preventDefault()
  event.dataTransfer.dropEffect = 'copy'
  isDragOver.value = true
}

// 拖动离开处理
function handleDragLeave(event: DragEvent) {
  // 检查是否真的离开了面板（而不是进入子元素）
  const relatedTarget = event.relatedTarget as HTMLElement | null
  if (!fileListRef.value?.contains(relatedTarget)) {
    isDragOver.value = false
  }
}

// 放置处理
function handleDrop(event: DragEvent) {
  event.preventDefault()
  isDragOver.value = false

  if (!event.dataTransfer) return

  const jsonData = event.dataTransfer.getData('application/json')
  if (!jsonData) return

  try {
    const dragData = JSON.parse(jsonData)
    const sourceType = dragData.sourceType as PanelType
    const items = dragData.items as TransferItem[]

    // 验证传输方向限制
    // 规则：FTP/机床 只能拖到本地，本地可以拖到 FTP/机床
    if (sourceType === props.type) {
      toast.warning('不能拖放到同一面板')
      return
    }

    if (sourceType === 'local') {
      // 本地 -> FTP 或 机床：允许
    } else {
      // FTP/机床 -> 只能到本地
      if (props.type !== 'local') {
        toast.warning('FTP/机床文件只能传输到本地')
        return
      }
    }

    // 检查目标面板是否已连接
    if (!isConnected.value) {
      toast.warning('目标面板未连接，请先连接服务器')
      return
    }

    // 执行传输
    if (items.length > 0) {
      transferContext.startTransfer(sourceType, props.type, items)
    }
  } catch (e) {
    toast.error('拖放操作失败')
  }
}

// 框选功能
function handleMouseDown(event: MouseEvent) {
  // 只响应左键，且不在文件项上点击时才启动框选
  if (event.button !== 0) return
  const target = event.target as HTMLElement
  if (target.closest('[data-file-item]')) return

  const container = fileListRef.value
  if (!container) return

  const rect = container.getBoundingClientRect()
  const x = event.clientX - rect.left + container.scrollLeft
  const y = event.clientY - rect.top + container.scrollTop

  selectionBox.value = { startX: x, startY: y, x, y, width: 0, height: 0 }
  isSelecting.value = true

  // 如果没有按住Ctrl/Cmd，清空之前的选择
  if (!event.ctrlKey && !event.metaKey) {
    selectedFiles.value = []
  }

  document.addEventListener('mousemove', handleMouseMove)
  document.addEventListener('mouseup', handleMouseUp)
}

function handleMouseMove(event: MouseEvent) {
  if (!isSelecting.value || !fileListRef.value) return

  const container = fileListRef.value
  const rect = container.getBoundingClientRect()
  const currentX = event.clientX - rect.left + container.scrollLeft
  const currentY = event.clientY - rect.top + container.scrollTop

  const startX = selectionBox.value.startX
  const startY = selectionBox.value.startY

  selectionBox.value.x = Math.min(startX, currentX)
  selectionBox.value.y = Math.min(startY, currentY)
  selectionBox.value.width = Math.abs(currentX - startX)
  selectionBox.value.height = Math.abs(currentY - startY)

  updateSelectionFromBox()
}

function handleMouseUp() {
  isSelecting.value = false
  document.removeEventListener('mousemove', handleMouseMove)
  document.removeEventListener('mouseup', handleMouseUp)
}

function updateSelectionFromBox() {
  if (!fileListRef.value) return

  const container = fileListRef.value
  const items = container.querySelectorAll('[data-file-item]')
  const box = selectionBox.value
  const newSelection: string[] = []

  items.forEach((item) => {
    const rect = item.getBoundingClientRect()
    const containerRect = container.getBoundingClientRect()

    // 计算元素相对于容器的位置（考虑滚动）
    const itemX = rect.left - containerRect.left + container.scrollLeft
    const itemY = rect.top - containerRect.top + container.scrollTop
    const itemRight = itemX + rect.width
    const itemBottom = itemY + rect.height

    // 检查是否与选择框相交
    const boxRight = box.x + box.width
    const boxBottom = box.y + box.height

    if (itemX < boxRight && itemRight > box.x && itemY < boxBottom && itemBottom > box.y) {
      const fileName = item.getAttribute('data-file-name')
      if (fileName) newSelection.push(fileName)
    }
  })

  selectedFiles.value = newSelection
}

onUnmounted(() => {
  document.removeEventListener('mousemove', handleMouseMove)
  document.removeEventListener('mouseup', handleMouseUp)
  document.removeEventListener('keydown', handleKeyDown)
})
</script>

<template>
  <GlassPanel class="flex flex-col h-full flex-1" :padding="'none'" :radius="'xl'">
    <!-- 标题栏 -->
    <div class="flex items-center justify-between px-4 py-3 border-b border-[#E5E7EB]">
      <div class="flex items-center gap-2">
        <component :is="panelIcon" :size="20" class="text-[var(--color-primary)]" />
        <span class="font-semibold text-[var(--color-text-primary)]">{{ title }}</span>
        <div
          v-if="isConnected && type !== 'local'"
          class="w-2 h-2 rounded-full bg-[var(--color-success)]"
        />
      </div>
      <GlassButton variant="icon" icon="RefreshCw" @click="handleRefresh" />
    </div>

    <!-- 服务器选择 -->
    <div v-if="showServerSelect" class="px-4 py-3 border-b border-[#E5E7EB]">
      <div class="flex gap-2">
        <GlassDropdown
          :modelValue="selectedServer"
          :options="serverOptions"
          placeholder="选择服务器..."
          class="flex-1"
          @update:modelValue="handleServerChange"
        />
        <GlassButton
          :variant="isConnected ? 'secondary' : 'primary'"
          :loading="isLoading"
          size="sm"
          @click="handleConnect"
        >
          {{ isConnected ? '断开' : '连接' }}
        </GlassButton>
      </div>
    </div>

    <!-- 路径导航 -->
    <div class="px-4 py-3 border-b border-[#E5E7EB]">
      <GlassInput
        v-model="currentPath"
        placeholder="路径"
        leftActionIcon="ArrowLeft"
        class="w-full"
        @leftAction="handleGoUp"
      />
    </div>

    <!-- 文件列表包装容器 -->
    <div class="flex-1 relative overflow-hidden">
      <!-- 滚动容器 -->
      <div
        ref="fileListRef"
        tabindex="0"
        class="absolute inset-0 overflow-y-auto p-3 select-none outline-none"
        :class="{ 'pointer-events-none': isLoading, 'ring-2 ring-[var(--color-primary)] ring-inset bg-[var(--color-primary)]/5': isDragOver }"
        @mousedown="handleMouseDown"
        @contextmenu="handleContextMenu"
        @focusin="handlePanelFocus"
        @focusout="handlePanelBlur"
        @dragover="handleDragOver"
        @dragleave="handleDragLeave"
        @drop="handleDrop"
      >
      <template v-if="files.length > 0">
        <template v-for="file in files" :key="file.name">
          <FolderListItem
            v-if="file.isDir"
            :name="file.name"
            :selected="selectedFiles.includes(file.name)"
            draggable="true"
            data-file-item
            :data-file-name="file.name"
            @click="toggleSelect(file.name, $event)"
            @open="handleFileClick(file)"
            @dragstart="handleDragStart($event, file)"
          />
          <FileListItem
            v-else
            :name="file.name"
            :size="file.size"
            :selected="selectedFiles.includes(file.name)"
            draggable="true"
            data-file-item
            :data-file-name="file.name"
            @click="toggleSelect(file.name, $event)"
            @dblclick="handleFileClick(file)"
            @dragstart="handleDragStart($event, file)"
          />
        </template>
      </template>
      <div v-else class="flex flex-col items-center justify-center h-full text-[var(--color-text-tertiary)]">
        <FolderOpen :size="48" class="mb-4 opacity-50" />
        <span class="text-sm">
          {{ isConnected ? '空目录' : '请先连接服务器' }}
        </span>
      </div>
      <!-- 框选视觉元素 -->
      <div
        v-if="isSelecting"
        class="absolute pointer-events-none border-2 border-[var(--color-primary)] bg-[var(--color-primary)]/10 rounded"
        :style="{
          left: selectionBox.x + 'px',
          top: selectionBox.y + 'px',
          width: selectionBox.width + 'px',
          height: selectionBox.height + 'px'
        }"
      />
      </div>
      <!-- Loading遮罩层 - 在滚动容器外面，固定覆盖 -->
      <div
        v-if="isLoading && files.length > 0"
        class="absolute inset-0 bg-white/60 backdrop-blur-sm flex items-center justify-center z-10"
      >
        <div class="flex flex-col items-center gap-2">
          <div class="w-8 h-8 border-3 border-[var(--color-primary)] border-t-transparent rounded-full animate-spin" />
          <span class="text-sm text-[var(--color-text-secondary)]">加载中...</span>
        </div>
      </div>
    </div>
  </GlassPanel>

  <!-- 切换服务器确认对话框 -->
  <GlassModal
    :visible="showSwitchConfirm"
    title="切换服务器"
    confirmText="确定切换"
    @confirm="confirmSwitchServer"
    @cancel="cancelSwitchServer"
  >
    <p class="text-[var(--color-text-secondary)]">
      当前正在传输文件，切换服务器将中断传输。确定要切换吗？
    </p>
  </GlassModal>

  <!-- 删除确认对话框 -->
  <GlassModal
    :visible="showDeleteConfirm"
    title="确认删除"
    confirmText="删除"
    @confirm="confirmDelete"
    @cancel="cancelDelete"
  >
    <p class="text-[var(--color-text-secondary)]">
      确定要删除以下 {{ filesToDelete.length }} 个项目吗？此操作不可恢复。
    </p>
    <ul class="mt-2 text-sm text-[var(--color-text-tertiary)] max-h-32 overflow-y-auto">
      <li v-for="name in filesToDelete" :key="name" class="truncate">• {{ name }}</li>
    </ul>
  </GlassModal>

  <!-- 右键菜单 -->
  <GlassContextMenu
    :visible="showContextMenu"
    :x="contextMenuX"
    :y="contextMenuY"
    :items="contextMenuItems"
    @select="handleContextMenuSelect"
    @close="closeContextMenu"
  />
</template>
