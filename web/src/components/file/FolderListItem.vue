<template>
  <div
    :class="itemClasses"
    @click="$emit('click', $event)"
    @dblclick="$emit('open')"
  >
    <Folder :size="20" class="text-[var(--color-warning)] flex-shrink-0" />
    <div class="flex-1 min-w-0 flex flex-col gap-0.5">
      <span class="text-sm font-medium text-[var(--color-text-primary)] truncate">
        {{ name }}
      </span>
      <span v-if="itemCount !== undefined" class="text-xs text-[var(--color-text-tertiary)]">
        {{ itemCount }} 项
      </span>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { Folder } from 'lucide-vue-next'

interface FolderListItemProps {
  name: string
  itemCount?: number
  selected?: boolean
}

const props = withDefaults(defineProps<FolderListItemProps>(), {
  selected: false
})

defineEmits<{
  click: [event: MouseEvent]
  open: []
}>()

const itemClasses = computed(() => [
  'flex items-center gap-[10px] px-3 py-2 rounded-[8px] overflow-hidden',
  'cursor-pointer transition-colors duration-150',
  props.selected
    ? 'bg-[var(--color-primary)]/10 border border-[var(--color-primary)]/20'
    : 'hover:bg-white/50 border border-transparent'
])
</script>
