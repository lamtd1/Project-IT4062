<<<<<<< HEAD
import React from 'react' // <--- THÊM DÒNG NÀY
import ReactDOM from 'react-dom/client'
import App from './App.jsx'
import './index.css' // (Nếu có)

ReactDOM.createRoot(document.getElementById('root')).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
)
=======
import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import './index.css'
import App from './App.jsx'

createRoot(document.getElementById('root')).render(
  <StrictMode>
    <App />
  </StrictMode>,
)
>>>>>>> 538ebfdff0f65edeb7e54ff177ef1015f3963a85
