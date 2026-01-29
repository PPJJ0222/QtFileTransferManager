<template>
  <div class="relative flex items-center">
    <!-- 左侧操作按钮 -->
    <button
      v-if="leftActionIcon"
      type="button"
      class="absolute left-1 top-1/2 -translate-y-1/2 w-8 h-8 flex items-center justify-center rounded-lg hover:bg-[var(--color-bg-tertiary)] transition-colors z-10"
      @click="emit('leftAction')"
    >
      <component
        :is="getIcon(leftActionIcon)"
        :size="18"
        class="text-[var(--color-text-secondary)]"
      />
    </button>
    <!-- 普通图标 -->
    <component
      v-if="icon && !leftActionIcon"
      :is="getIcon(icon)"
      :size="16"
      class="absolute left-3 top-1/2 -translate-y-1/2 text-[var(--color-text-tertiary)]"
    />
    <input
      :type="type"
      :value="modelValue"
      :placeholder="placeholder"
      :disabled="disabled"
      :class="inputClasses"
      @input="handleInput"
    />
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import * as icons from 'lucide-vue-next'

interface GlassInputProps {
  modelValue?: string
  placeholder?: string
  type?: 'text' | 'password' | 'search'
  icon?: string
  leftActionIcon?: string
  disabled?: boolean
}

const props = withDefaults(defineProps<GlassInputProps>(), {
  type: 'text',
  disabled: false
})

const emit = defineEmits<{
  'update:modelValue': [value: string]
  'leftAction': []
}>()

function getIcon(name: string) {
  const iconName = name.replace(/-./g, x => x[1].toUpperCase())
  const pascalName = iconName.charAt(0).toUpperCase() + iconName.slice(1)
  return (icons as Record<string, unknown>)[pascalName] || null
}

function handleInput(event: Event) {
  const target = event.target as HTMLInputElement
  emit('update:modelValue', target.value)
}

const inputClasses = computed(() => [
  'w-full glass-input rounded-[12px] h-10 text-sm',
  'text-[var(--color-text-primary)] placeholder:text-[var(--color-text-tertiary)]',
  'focus:border-[var(--color-primary)] focus:outline-none',
  'transition-colors duration-200',
  props.leftActionIcon ? 'pl-11 pr-4' : props.icon ? 'pl-10 pr-4' : 'px-3',
  props.disabled && 'opacity-50 cursor-not-allowed'
])
</script>
