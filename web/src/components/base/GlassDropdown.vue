<template>
  <div class="relative" ref="dropdownRef">
    <!-- 触发器 -->
    <button
      type="button"
      :class="triggerClasses"
      :disabled="disabled"
      @click="toggle"
    >
      <span class="text-[var(--color-text-primary)] text-[13px] font-medium truncate">
        {{ selectedLabel || placeholder }}
      </span>
      <ChevronDown
        :size="16"
        :class="[
          'text-[var(--color-text-tertiary)] transition-transform duration-200',
          isOpen && 'rotate-180'
        ]"
      />
    </button>

    <!-- 下拉菜单 -->
    <Transition
      enter-active-class="transition duration-150 ease-out"
      enter-from-class="opacity-0 -translate-y-1"
      enter-to-class="opacity-100 translate-y-0"
      leave-active-class="transition duration-100 ease-in"
      leave-from-class="opacity-100 translate-y-0"
      leave-to-class="opacity-0 -translate-y-1"
    >
      <div v-if="isOpen" :class="menuClasses">
        <div
          v-for="option in options"
          :key="option.value"
          :class="getOptionClasses(option)"
          @click="selectOption(option)"
        >
          <Check
            v-if="option.value === modelValue"
            :size="16"
            class="text-[var(--color-primary)] shrink-0"
          />
          <span v-else class="w-4 shrink-0" />
          <span class="truncate">{{ option.label }}</span>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { ChevronDown, Check } from 'lucide-vue-next'

export interface DropdownOption {
  label: string
  value: string | number
}

interface GlassDropdownProps {
  modelValue?: string | number
  options: DropdownOption[]
  placeholder?: string
  disabled?: boolean
}

const props = withDefaults(defineProps<GlassDropdownProps>(), {
  placeholder: '请选择',
  disabled: false
})

const emit = defineEmits<{
  'update:modelValue': [value: string | number]
}>()

const isOpen = ref(false)
const dropdownRef = ref<HTMLElement | null>(null)

const selectedLabel = computed(() => {
  const selected = props.options.find(opt => opt.value === props.modelValue)
  return selected?.label
})

const triggerClasses = computed(() => [
  'w-full flex items-center justify-between gap-2',
  'h-10 px-3 rounded-[var(--radius-md)]',
  'glass-dropdown-trigger',
  'transition-all duration-200',
  'focus:outline-none focus:border-[var(--color-primary)]',
  props.disabled && 'opacity-50 cursor-not-allowed'
])

const menuClasses = computed(() => [
  'absolute z-50 w-full mt-1',
  'p-1.5 rounded-[var(--radius-md)]',
  'glass-dropdown-menu',
  'max-h-[200px] overflow-y-auto custom-scrollbar'
])

function getOptionClasses(option: DropdownOption) {
  const isSelected = option.value === props.modelValue
  return [
    'flex items-center gap-2.5 px-3 py-2',
    'rounded-[var(--radius-sm)] cursor-pointer',
    'text-[14px] text-[var(--color-text-primary)]',
    'transition-colors duration-150',
    isSelected
      ? 'bg-[var(--color-primary)]/10'
      : 'hover:bg-black/5'
  ]
}

function toggle() {
  if (!props.disabled) {
    isOpen.value = !isOpen.value
  }
}

function selectOption(option: DropdownOption) {
  emit('update:modelValue', option.value)
  isOpen.value = false
}

function handleClickOutside(event: MouseEvent) {
  if (dropdownRef.value && !dropdownRef.value.contains(event.target as Node)) {
    isOpen.value = false
  }
}

onMounted(() => {
  document.addEventListener('click', handleClickOutside)
})

onUnmounted(() => {
  document.removeEventListener('click', handleClickOutside)
})
</script>
