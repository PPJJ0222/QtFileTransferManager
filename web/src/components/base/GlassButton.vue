<template>
  <button
    :class="buttonClasses"
    :disabled="disabled || loading"
    @click="$emit('click', $event)"
  >
    <span v-if="loading" class="animate-spin">
      <Loader2 :size="16" />
    </span>
    <template v-else>
      <component
        v-if="icon"
        :is="getIcon(icon)"
        :size="variant === 'icon' ? 18 : 16"
        :class="variant === 'primary' ? 'text-white' : 'text-[var(--color-text-primary)]'"
      />
      <span v-if="variant !== 'icon'">
        <slot />
      </span>
    </template>
  </button>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import * as icons from 'lucide-vue-next'
import { Loader2 } from 'lucide-vue-next'

interface GlassButtonProps {
  variant?: 'primary' | 'secondary' | 'icon'
  size?: 'sm' | 'md' | 'lg'
  disabled?: boolean
  loading?: boolean
  icon?: string
}

const props = withDefaults(defineProps<GlassButtonProps>(), {
  variant: 'primary',
  size: 'md',
  disabled: false,
  loading: false
})

defineEmits<{
  click: [event: MouseEvent]
}>()

function getIcon(name: string) {
  const iconName = name.replace(/-./g, x => x[1].toUpperCase())
  const pascalName = iconName.charAt(0).toUpperCase() + iconName.slice(1)
  return (icons as Record<string, unknown>)[pascalName] || null
}

const buttonClasses = computed(() => {
  const base = 'inline-flex items-center justify-center gap-2 transition-all duration-200 font-medium cursor-pointer'

  const variants: Record<string, string> = {
    primary: 'bg-gradient-to-b from-[#3B82F6] to-[#2563EB] text-white hover:from-[#2563EB] hover:to-[#1D4ED8] shadow-[0_2px_8px_rgba(59,130,246,0.25)]',
    secondary: 'glass-light text-[var(--color-text-primary)] hover:bg-white/90 border border-[#E5E7EB] shadow-[0_1px_4px_rgba(0,0,0,0.06)]',
    icon: 'glass-light hover:bg-white/90 border border-[#E5E7EB] shadow-[0_1px_4px_rgba(0,0,0,0.06)]'
  }

  const sizes: Record<string, string> = {
    sm: 'px-3 py-1.5 text-sm rounded-[8px]',
    md: 'px-5 py-[10px] text-sm rounded-[12px]',
    lg: 'px-6 py-3 text-base rounded-[12px]'
  }

  return [
    base,
    variants[props.variant],
    props.variant === 'icon' ? 'w-9 h-9 rounded-[8px]' : sizes[props.size],
    (props.disabled || props.loading) && 'opacity-50 cursor-not-allowed'
  ]
})
</script>
