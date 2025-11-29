import React from 'react' // <--- THÊM DÒNG NÀY
import ReactDOM from 'react-dom/client'
import App from './App.jsx'
import './index.css' // (Nếu có)

ReactDOM.createRoot(document.getElementById('root')).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
)