<script setup lang="ts">
import { computed } from 'vue'
import { FileUp, X } from 'lucide-vue-next'
import { GlassButton } from './base'

const props = defineProps<{
  progress: number
  text: string
  visible: boolean
  // 批量传输相关
  currentIndex?: number
  totalCount?: number
}>()

const emit = defineEmits<{
  cancel: []
}>()

// 是否为批量传输
const isBatch = computed(() => (props.totalCount ?? 0) > 1)

// 整体进度百分比
const overallProgress = computed(() => {
  if (!isBatch.value || !props.totalCount) return props.progress
  // 已完成的文件 + 当前文件的进度
  const completed = (props.currentIndex ?? 0) - 1
  const currentContribution = props.progress / props.totalCount
  return Math.round((completed / props.totalCount) * 100 + currentContribution)
})
</script>

<template>
  <Transition name="slide-up">
    <div v-if="visible" class="glass-panel rounded-[16px] p-3">
      <div class="flex items-center justify-between mb-2">
        <div class="flex items-center gap-2 flex-1 min-w-0">
          <FileUp :size="18" class="text-[var(--color-primary)] flex-shrink-0" />
          <span class="text-sm font-medium text-[var(--color-text-primary)] truncate">
            {{ text }}
          </span>
        </div>
        <div class="flex items-center gap-3 flex-shrink-0">
          <!-- 批量传输显示整体进度 -->
          <template v-if="isBatch">
            <span class="text-xs text-[var(--color-text-secondary)]">
              {{ currentIndex }}/{{ totalCount }}
            </span>
            <span class="text-sm font-semibold text-[var(--color-primary)]">
              {{ overallProgress }}%
            </span>
          </template>
          <!-- 单文件传输显示当前进度 -->
          <span v-else class="text-sm font-semibold text-[var(--color-primary)]">
            {{ progress }}%
          </span>
          <GlassButton variant="icon" @click="emit('cancel')">
            <X :size="16" />
          </GlassButton>
        </div>
      </div>
      <!-- 批量传输：双进度条 -->
      <template v-if="isBatch">
        <!-- 整体进度条 -->
        <div class="h-2 bg-[#E5E7EB] rounded-full overflow-hidden mb-1.5">
          <div
            class="h-full bg-gradient-to-r from-[var(--color-primary)] to-[#60A5FA] rounded-full transition-all duration-300"
            :style="{ width: `${overallProgress}%` }"
          />
        </div>
        <!-- 当前文件进度条（细一点） -->
        <div class="h-1 bg-[#E5E7EB]/50 rounded-full overflow-hidden">
          <div
            class="h-full bg-[var(--color-primary)]/50 rounded-full transition-all duration-150"
            :style="{ width: `${progress}%` }"
          />
        </div>
      </template>
      <!-- 单文件传输：单进度条 -->
      <div v-else class="h-2 bg-[#E5E7EB] rounded-full overflow-hidden">
        <div
          class="h-full bg-gradient-to-r from-[var(--color-primary)] to-[#60A5FA] rounded-full transition-all duration-300"
          :style="{ width: `${progress}%` }"
        />
      </div>
    </div>
  </Transition>
</template>

<style scoped>
.slide-up-enter-active,
.slide-up-leave-active {
  transition: all 0.3s ease;
}

.slide-up-enter-from,
.slide-up-leave-to {
  opacity: 0;
  transform: translateY(20px);
}
</style>
