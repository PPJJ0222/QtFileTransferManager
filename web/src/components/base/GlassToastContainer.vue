<script setup lang="ts">
import { useToast } from '../../composables/useToast'
import GlassToast from './GlassToast.vue'

const { toasts, removeToast } = useToast()
</script>

<template>
  <Teleport to="body">
    <div class="toast-container">
      <TransitionGroup name="toast-slide">
        <GlassToast
          v-for="toast in toasts"
          :key="toast.id"
          :type="toast.type"
          :message="toast.message"
          :closable="true"
          @close="removeToast(toast.id)"
        />
      </TransitionGroup>
    </div>
  </Teleport>
</template>

<style scoped>
.toast-container {
  position: fixed;
  top: 20px;
  right: 20px;
  z-index: 9999;
  display: flex;
  flex-direction: column;
  gap: 12px;
  pointer-events: none;
}

.toast-container > :deep(*) {
  pointer-events: auto;
  will-change: transform, opacity;
}

/* 侧滑动画 - 使用GPU加速 */
.toast-slide-enter-active {
  transition: transform 0.35s cubic-bezier(0.21, 1.02, 0.73, 1),
              opacity 0.25s ease-out;
}

.toast-slide-leave-active {
  transition: transform 0.25s cubic-bezier(0.06, 0.71, 0.55, 1),
              opacity 0.2s ease-in;
  position: absolute;
  right: 0;
}

.toast-slide-enter-from {
  opacity: 0;
  transform: translate3d(120%, 0, 0);
}

.toast-slide-leave-to {
  opacity: 0;
  transform: translate3d(100%, 0, 0);
}

.toast-slide-move {
  transition: transform 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}
</style>
