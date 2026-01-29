<template>
  <div :class="panelClasses">
    <slot />
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'

interface GlassPanelProps {
  padding?: 'none' | 'xs' | 'sm' | 'md' | 'lg' | 'xl'
  radius?: 'sm' | 'md' | 'lg' | 'xl'
  shadow?: boolean
  border?: boolean
}

const props = withDefaults(defineProps<GlassPanelProps>(), {
  padding: 'lg',
  radius: 'xl',
  shadow: true,
  border: true
})

const paddingMap: Record<string, string> = {
  none: 'p-0',
  xs: 'p-[4px]',
  sm: 'p-[8px]',
  md: 'p-[12px]',
  lg: 'p-[16px]',
  xl: 'p-[24px]'
}

const radiusMap: Record<string, string> = {
  sm: 'rounded-[8px]',
  md: 'rounded-[12px]',
  lg: 'rounded-[16px]',
  xl: 'rounded-[20px]'
}

const panelClasses = computed(() => [
  'glass-panel',
  paddingMap[props.padding],
  radiusMap[props.radius],
  props.shadow && 'shadow-[0_4px_24px_rgba(0,0,0,0.06)]',
  props.border && 'border border-white/25'
])
</script>
