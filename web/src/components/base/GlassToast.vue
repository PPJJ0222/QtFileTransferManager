<script setup lang="ts">
import { computed } from 'vue'
import { X, CheckCircle, AlertTriangle, XCircle, Info } from 'lucide-vue-next'

const props = withDefaults(defineProps<{
  type?: 'success' | 'warning' | 'error' | 'info'
  message: string
  closable?: boolean
}>(), {
  type: 'info',
  closable: true
})

const emit = defineEmits<{
  close: []
}>()

// 图标映射
const iconMap = {
  success: CheckCircle,
  warning: AlertTriangle,
  error: XCircle,
  info: Info
}

// 颜色映射（使用CSS变量）
const colorMap = {
  success: 'var(--color-success)',
  warning: 'var(--color-warning)',
  error: 'var(--color-error)',
  info: 'var(--color-primary)'
}

const IconComponent = computed(() => iconMap[props.type])
const iconColor = computed(() => colorMap[props.type])
</script>

<template>
  <div class="glass-toast">
    <!-- 左侧色条 -->
    <div
      class="toast-indicator"
      :style="{ backgroundColor: iconColor }"
    />

    <!-- 图标 -->
    <component
      :is="IconComponent"
      :size="20"
      :style="{ color: iconColor }"
      class="flex-shrink-0"
    />

    <!-- 消息内容 -->
    <span class="toast-message">{{ message }}</span>

    <!-- 关闭按钮 -->
    <button
      v-if="closable"
      class="toast-close"
      @click="emit('close')"
    >
      <X :size="16" />
    </button>
  </div>
</template>

<style scoped>
.glass-toast {
  display: flex;
  align-items: center;
  gap: 12px;
  min-width: 280px;
  max-width: 400px;
  padding: 12px 16px;
  background: rgba(255, 255, 255, 0.95);
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  border: 1px solid rgba(255, 255, 255, 0.5);
  border-radius: 12px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.12);
  position: relative;
  overflow: hidden;
}

.toast-indicator {
  position: absolute;
  left: 0;
  top: 0;
  bottom: 0;
  width: 4px;
}

.toast-message {
  flex: 1;
  font-size: 14px;
  color: var(--color-text-primary);
  line-height: 1.4;
}

.toast-close {
  flex-shrink: 0;
  padding: 4px;
  border-radius: 6px;
  color: var(--color-text-tertiary);
  cursor: pointer;
  transition: all 0.2s;
  background: transparent;
  border: none;
}

.toast-close:hover {
  background: rgba(0, 0, 0, 0.05);
  color: var(--color-text-secondary);
}
</style>
