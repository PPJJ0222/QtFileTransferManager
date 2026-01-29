<script setup lang="ts">
import { ref, watch, onUnmounted } from 'vue'

export interface ContextMenuItem {
  label: string
  action: string
  icon?: string
}

const props = defineProps<{
  visible: boolean
  x: number
  y: number
  items: ContextMenuItem[]
}>()

const emit = defineEmits<{
  select: [action: string]
  close: []
}>()

const menuRef = ref<HTMLElement | null>(null)
let pendingTimer: number | null = null

function handleClickOutside(event: MouseEvent) {
  if (menuRef.value && !menuRef.value.contains(event.target as Node)) {
    emit('close')
  }
}

function addListeners() {
  document.addEventListener('click', handleClickOutside)
  document.addEventListener('contextmenu', handleClickOutside)
}

function removeListeners() {
  document.removeEventListener('click', handleClickOutside)
  document.removeEventListener('contextmenu', handleClickOutside)
}

// 监听 visible 变化，延迟添加/移除事件监听
watch(() => props.visible, (visible) => {
  if (visible) {
    // 清除之前可能存在的定时器
    if (pendingTimer !== null) {
      clearTimeout(pendingTimer)
    }
    // 使用 setTimeout 延迟添加监听
    pendingTimer = window.setTimeout(() => {
      // 再次检查 visible 状态，防止在延迟期间状态已改变
      if (props.visible) {
        addListeners()
      }
      pendingTimer = null
    }, 0)
  } else {
    // 清除定时器
    if (pendingTimer !== null) {
      clearTimeout(pendingTimer)
      pendingTimer = null
    }
    removeListeners()
  }
})

onUnmounted(() => {
  if (pendingTimer !== null) {
    clearTimeout(pendingTimer)
    pendingTimer = null
  }
  removeListeners()
})
</script>

<template>
  <Teleport to="body">
    <Transition
      enter-active-class="transition duration-100 ease-out"
      enter-from-class="opacity-0 scale-95"
      enter-to-class="opacity-100 scale-100"
      leave-active-class="transition duration-75 ease-in"
      leave-from-class="opacity-100 scale-100"
      leave-to-class="opacity-0 scale-95"
    >
      <div
        v-if="visible"
        ref="menuRef"
        class="fixed z-50 min-w-[160px] p-1.5 rounded-lg bg-white/95 backdrop-blur-md shadow-lg border border-gray-200/50 select-none"
        :style="{ left: x + 'px', top: y + 'px' }"
        @contextmenu.prevent
      >
        <button
          v-for="item in items"
          :key="item.action"
          class="w-full px-3 py-2 text-left text-sm text-[var(--color-text-primary)] hover:bg-[var(--color-primary)]/10 transition-colors rounded-md"
          @click="emit('select', item.action)"
        >
          {{ item.label }}
        </button>
      </div>
    </Transition>
  </Teleport>
</template>
