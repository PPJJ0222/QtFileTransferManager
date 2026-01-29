<template>
  <div
    :class="itemClasses"
    @click="$emit('click', $event)"
    @dblclick="$emit('dblclick')"
  >
    <File :size="20" class="text-[var(--color-primary)] flex-shrink-0" />
    <div class="flex-1 min-w-0 flex flex-col gap-0.5">
      <span class="text-sm font-medium text-[var(--color-text-primary)] truncate">
        {{ name }}
      </span>
      <span class="text-xs text-[var(--color-text-tertiary)]">
        {{ formatSize(size) }}
      </span>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { File } from 'lucide-vue-next'

interface FileListItemProps {
  name: string
  size: number
  selected?: boolean
}

const props = withDefaults(defineProps<FileListItemProps>(), {
  selected: false
})

defineEmits<{
  click: [event: MouseEvent]
  dblclick: []
}>()

const itemClasses = computed(() => [
  'flex items-center gap-[10px] px-3 py-2 rounded-[8px] overflow-hidden',
  'cursor-pointer transition-colors duration-150',
  props.selected
    ? 'bg-[var(--color-primary)]/10 border border-[var(--color-primary)]/20'
    : 'hover:bg-white/50 border border-transparent'
])

function formatSize(size: number): string {
  if (size < 1024) return `${size} B`
  if (size < 1024 * 1024) return `${(size / 1024).toFixed(1)} KB`
  return `${(size / 1024 / 1024).toFixed(1)} MB`
}
</script>
