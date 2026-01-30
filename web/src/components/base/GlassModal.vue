<script setup lang="ts">
import { X } from 'lucide-vue-next'
import GlassButton from './GlassButton.vue'

defineProps<{
  visible: boolean
  title?: string
  confirmText?: string
  cancelText?: string
  confirmVariant?: 'primary' | 'secondary'
  width?: string
}>()

const emit = defineEmits<{
  confirm: []
  cancel: []
}>()
</script>

<template>
  <Teleport to="body">
    <Transition name="modal">
      <div
        v-if="visible"
        class="fixed inset-0 z-50 flex items-center justify-center"
      >
        <!-- 遮罩层 -->
        <div
          class="absolute inset-0 bg-black/30 backdrop-blur-sm"
          @click="emit('cancel')"
        />
        <!-- 对话框 -->
        <div
          class="relative bg-white/90 backdrop-blur-md rounded-2xl shadow-xl border border-white/50 max-w-[90vw]"
          :style="{ width: width || '400px' }"
        >
          <!-- 标题栏 -->
          <div class="flex items-center justify-between px-5 py-4 border-b border-gray-200/50">
            <h3 class="text-lg font-semibold text-[var(--color-text-primary)]">
              {{ title || '提示' }}
            </h3>
            <button
              class="p-1 rounded-lg hover:bg-gray-100 transition-colors"
              @click="emit('cancel')"
            >
              <X :size="20" class="text-gray-500" />
            </button>
          </div>
          <!-- 内容 -->
          <div class="px-5 py-4">
            <slot />
          </div>
          <!-- 按钮 -->
          <div class="flex justify-end gap-3 px-5 py-4 border-t border-gray-200/50">
            <GlassButton
              v-if="cancelText !== ''"
              variant="secondary"
              @click="emit('cancel')"
            >
              {{ cancelText || '取消' }}
            </GlassButton>
            <GlassButton
              :variant="confirmVariant || 'primary'"
              @click="emit('confirm')"
            >
              {{ confirmText || '确定' }}
            </GlassButton>
          </div>
        </div>
      </div>
    </Transition>
  </Teleport>
</template>

<style scoped>
.modal-enter-active,
.modal-leave-active {
  transition: opacity 0.2s ease;
}

.modal-enter-active .relative,
.modal-leave-active .relative {
  transition: transform 0.2s ease;
}

.modal-enter-from,
.modal-leave-to {
  opacity: 0;
}

.modal-enter-from .relative {
  transform: scale(0.95);
}

.modal-leave-to .relative {
  transform: scale(0.95);
}
</style>
