import { createApp } from 'vue'
import App from './App.vue'
import { initBridge } from './bridge'

// 导入全局样式
import './assets/styles/main.css'

// 初始化 Bridge 后再挂载 Vue
initBridge().then(() => {
  createApp(App).mount('#app')
})
