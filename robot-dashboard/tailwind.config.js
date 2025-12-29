/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        'nightfall': {
          'primary': '#0f172a',
          'secondary': '#1e293b',
          'accent': '#3b82f6',
          'success': '#10b981',
          'warning': '#f59e0b',
          'error': '#ef4444',
        },
        'robot': {
          'brain': '#8b5cf6',
          'motors': '#06b6d4',
          'vision': '#84cc16',
        }
      },
      animation: {
        'pulse-slow': 'pulse 3s cubic-bezier(0.4, 0, 0.6, 1) infinite',
        'bounce-slow': 'bounce 2s infinite',
      },
      fontFamily: {
        'mono': ['JetBrains Mono', 'Consolas', 'Monaco', 'monospace'],
      },
      screens: {
        'xs': '475px',
      }
    },
  },
  plugins: [],
}