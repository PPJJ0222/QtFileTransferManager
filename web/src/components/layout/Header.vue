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
        <GlassButton variant="secondary" size="sm" @click="$emit('machineConfig')">
          <Server :size="16" class="mr-1.5" />
          机床配置
        </GlassButton>
      </slot>
    </div>
  </header>
</template>

<script setup lang="ts">
import { HardDrive, Server } from 'lucide-vue-next'
import { GlassButton } from '@/components/base'

interface HeaderProps {
  title?: string
}

withDefaults(defineProps<HeaderProps>(), {
  title: '文件传输管理器'
})

defineEmits<{
  refresh: []
  machineConfig: []
}>()
</script>
