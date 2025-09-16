import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  test: {
    globals: true,
    environment: 'jsdom',
    setupFiles: ['./src/test/setup.ts'],
  },
  server: {
    port: parseInt(process.env.PORT || '3102'),
    host: true,
    proxy: {
      '/api': {
        target: 'http://localhost:3001',  // Changed from 3002 to 3001 to match backend
        changeOrigin: true,
      },
      '/ws': {
        target: 'ws://localhost:3001',    // Changed from 3002 to 3001 to match backend
        ws: true,
      },
    },
  },
  build: {
    outDir: 'dist',
    sourcemap: true,
  },
  resolve: {
    alias: {
      '@': '/src',
    },
  },
})
