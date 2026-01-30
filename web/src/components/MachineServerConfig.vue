<script setup lang="ts">
import { ref, watch, onMounted } from 'vue'
import { Plus, Pencil, Trash2, Server } from 'lucide-vue-next'
import { GlassButton, GlassInput, GlassModal } from './base'
import { getBridge, onBridgeReady } from '@/bridge'
import { useToast } from '@/composables/useToast'
import type { ServerConfig } from '@/types'

const props = defineProps<{
  visible: boolean
}>()

const emit = defineEmits<{
  close: []
}>()

const toast = useToast()

// 服务器列表
const servers = ref<ServerConfig[]>([])

// 编辑模式: none | add | edit
const editMode = ref<'none' | 'add' | 'edit'>('none')

// 编辑时的原始名称
const originalName = ref('')

// 表单数据
const formData = ref({
  name: '',
  host: '',
  port: '21',
  user: '',
  password: ''
})

// 删除确认弹窗
const showDeleteConfirm = ref(false)
const deleteTargetName = ref('')

// 加载服务器列表
function loadServers() {
  const bridge = getBridge()
  if (bridge) {
    bridge.loadMachineServers()
  }
}

// 监听弹窗打开
watch(() => props.visible, (visible) => {
  if (visible) {
    loadServers()
    resetForm()
  }
})

// 重置表单
function resetForm() {
  editMode.value = 'none'
  originalName.value = ''
  formData.value = {
    name: '',
    host: '',
    port: '21',
    user: '',
    password: ''
  }
}

// 开始新增
function startAdd() {
  resetForm()
  editMode.value = 'add'
}

// 开始编辑
function startEdit(server: ServerConfig) {
  editMode.value = 'edit'
  originalName.value = server.name
  formData.value = {
    name: server.name,
    host: server.host,
    port: String(server.port),
    user: server.user,
    password: server.password
  }
}

// 取消编辑
function cancelEdit() {
  resetForm()
}

// 保存服务器
function saveServer() {
  // 表单验证
  if (!formData.value.name.trim()) {
    toast.error('请输入服务器名称')
    return
  }
  if (!formData.value.host.trim()) {
    toast.error('请输入主机地址')
    return
  }
  const portNum = parseInt(formData.value.port, 10)
  if (isNaN(portNum) || portNum < 1 || portNum > 65535) {
    toast.error('端口号必须在 1-65535 之间')
    return
  }

  const bridge = getBridge()
  if (!bridge) return

  const config: ServerConfig = {
    name: formData.value.name.trim(),
    host: formData.value.host.trim(),
    port: portNum,
    user: formData.value.user.trim(),
    password: formData.value.password
  }

  if (editMode.value === 'add') {
    // 检查名称是否重复
    if (servers.value.some(s => s.name === config.name)) {
      toast.error('服务器名称已存在')
      return
    }
    bridge.saveMachineServer(config)
    toast.success('服务器添加成功')
  } else if (editMode.value === 'edit') {
    // 检查名称是否与其他服务器重复
    if (config.name !== originalName.value &&
        servers.value.some(s => s.name === config.name)) {
      toast.error('服务器名称已存在')
      return
    }
    bridge.updateMachineServer(originalName.value, config)
    toast.success('服务器更新成功')
  }

  resetForm()
  // 延迟刷新列表
  setTimeout(() => loadServers(), 100)
}

// 确认删除
function confirmDelete(name: string) {
  deleteTargetName.value = name
  showDeleteConfirm.value = true
}

// 执行删除
function doDelete() {
  const bridge = getBridge()
  if (bridge) {
    bridge.deleteMachineServer(deleteTargetName.value)
    toast.success('服务器删除成功')
    setTimeout(() => loadServers(), 100)
  }
  showDeleteConfirm.value = false
  deleteTargetName.value = ''
}

// 关闭弹窗
function handleClose() {
  resetForm()
  emit('close')
}

// 监听 Bridge 信号
onMounted(() => {
  onBridgeReady(() => {
    const bridge = getBridge() as any
    if (bridge?.machineServersLoaded?.connect) {
      bridge.machineServersLoaded.connect((list: ServerConfig[]) => {
        servers.value = list
      })
    }
  })
})
</script>

<template>
  <GlassModal
    :visible="visible"
    title="机床服务器配置"
    width="520px"
    confirm-text="关闭"
    cancel-text=""
    @confirm="handleClose"
    @cancel="handleClose"
  >
    <div class="space-y-4">
      <!-- 服务器列表标题 -->
      <div class="flex items-center justify-between">
        <span class="text-sm font-medium text-[var(--color-text-secondary)]">
          服务器列表
        </span>
        <GlassButton
          v-if="editMode === 'none'"
          variant="secondary"
          size="sm"
          @click="startAdd"
        >
          <Plus :size="14" class="mr-1" />
          新增
        </GlassButton>
      </div>

      <!-- 服务器列表 -->
      <div
        v-if="servers.length > 0 && editMode === 'none'"
        class="space-y-2 max-h-[200px] overflow-y-auto"
      >
        <div
          v-for="server in servers"
          :key="server.name"
          class="flex items-center justify-between p-3 rounded-xl bg-white/50 border border-gray-200/50"
        >
          <div class="flex items-center gap-3">
            <Server :size="18" class="text-[var(--color-primary)]" />
            <div>
              <div class="text-sm font-medium text-[var(--color-text-primary)]">
                {{ server.name }}
              </div>
              <div class="text-xs text-[var(--color-text-tertiary)]">
                {{ server.host }}:{{ server.port }}
              </div>
            </div>
          </div>
          <div class="flex items-center gap-1">
            <button
              class="p-1.5 rounded-lg hover:bg-gray-100 transition-colors"
              title="编辑"
              @click="startEdit(server)"
            >
              <Pencil :size="14" class="text-[var(--color-text-secondary)]" />
            </button>
            <button
              class="p-1.5 rounded-lg hover:bg-red-50 transition-colors"
              title="删除"
              @click="confirmDelete(server.name)"
            >
              <Trash2 :size="14" class="text-[var(--color-error)]" />
            </button>
          </div>
        </div>
      </div>

      <!-- 空状态 -->
      <div
        v-if="servers.length === 0 && editMode === 'none'"
        class="py-8 text-center text-sm text-[var(--color-text-tertiary)]"
      >
        暂无机床服务器，点击"新增"添加
      </div>

      <!-- 编辑表单 -->
      <div v-if="editMode !== 'none'" class="space-y-3 pt-2 border-t border-gray-200/50">
        <div class="text-sm font-medium text-[var(--color-text-secondary)]">
          {{ editMode === 'add' ? '新增服务器' : '编辑服务器' }}
        </div>

        <div class="grid grid-cols-2 gap-3">
          <div class="col-span-2">
            <label class="block text-xs text-[var(--color-text-tertiary)] mb-1">
              名称 <span class="text-[var(--color-error)]">*</span>
            </label>
            <GlassInput
              v-model="formData.name"
              placeholder="输入服务器名称"
            />
          </div>

          <div>
            <label class="block text-xs text-[var(--color-text-tertiary)] mb-1">
              主机地址 <span class="text-[var(--color-error)]">*</span>
            </label>
            <GlassInput
              v-model="formData.host"
              placeholder="192.168.1.100"
            />
          </div>

          <div>
            <label class="block text-xs text-[var(--color-text-tertiary)] mb-1">
              端口
            </label>
            <GlassInput
              v-model="formData.port"
              placeholder="21"
            />
          </div>

          <div>
            <label class="block text-xs text-[var(--color-text-tertiary)] mb-1">
              用户名
            </label>
            <GlassInput
              v-model="formData.user"
              placeholder="输入用户名"
            />
          </div>

          <div>
            <label class="block text-xs text-[var(--color-text-tertiary)] mb-1">
              密码
            </label>
            <GlassInput
              v-model="formData.password"
              type="password"
              placeholder="输入密码"
            />
          </div>
        </div>

        <div class="flex justify-end gap-2 pt-2">
          <GlassButton variant="secondary" size="sm" @click="cancelEdit">
            取消
          </GlassButton>
          <GlassButton variant="primary" size="sm" @click="saveServer">
            保存
          </GlassButton>
        </div>
      </div>
    </div>
  </GlassModal>

  <!-- 删除确认弹窗 -->
  <GlassModal
    :visible="showDeleteConfirm"
    title="确认删除"
    confirm-text="删除"
    confirm-variant="primary"
    @confirm="doDelete"
    @cancel="showDeleteConfirm = false"
  >
    <p class="text-sm text-[var(--color-text-secondary)]">
      确定要删除服务器 "{{ deleteTargetName }}" 吗？此操作不可撤销。
    </p>
  </GlassModal>
</template>
