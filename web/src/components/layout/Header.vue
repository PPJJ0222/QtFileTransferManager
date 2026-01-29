<template>
  <header class="glass-panel flex items-center justify-between px-5 h-14 rounded-[16px]">
    <div class="flex items-center gap-3">
      <HardDrive :size="24" class="text-[var(--color-primary)]" />
      <h1 class="text-base font-semibold text-[var(--color-text-primary)]">
        {{ title }}
      </h1>
    </div>
    <div class="flex items-center gap-2">
      <slot name="actions">
        <GlassButton variant="icon" icon="Settings" @click="$emit('settings')" />
      </slot>
    </div>
  </header>
</template>

<script setup lang="ts">
import { HardDrive } from 'lucide-vue-next'
import { GlassButton } from '@/components/base'

interface HeaderProps {
  title?: string
}

withDefaults(defineProps<HeaderProps>(), {
  title: '文件传输管理器'
})

defineEmits<{
  refresh: []
  settings: []
}>()
</script>
