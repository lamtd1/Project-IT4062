import React, { useEffect, useState } from 'react';
import io from 'socket.io-client';
import { BrowserRouter, Routes, Route, useNavigate } from 'react-router-dom';
import Login from './features/cong_vao/Login';
import Lobby from './features/truong_quay/Lobby';
import GameScreen from './features/ghe_nong/GameScreen';

// --- CONFIGURATION ---
const socket = io('http://localhost:4000');

const OPS = {
  LOGIN: 0x01,
  REGISTER: 0x02,
  LOGIN_SUCCESS: 0x03,
  LOGIN_FAILED: 0x04,
  REGISTER_SUCCESS: 0x05,
  REGISTER_FAILED: 0x06,
};

// --- MAIN LOGIC CONTAINER ---

const GameContent = () => {
  const navigate = useNavigate();
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  // eslint-disable-next-line no-unused-vars
  const [status, setStatus] = useState({ msg: '', type: '' });

  useEffect(() => {
    socket.on('server_to_client', (data) => {
      const view = new Uint8Array(data);
      const opcode = view[0];

      if (opcode === OPS.LOGIN_SUCCESS) {
        setStatus({ msg: "Đăng nhập thành công!", type: 'success' });
        setTimeout(() => navigate('/home'), 1000);
      }
      else if (opcode === OPS.LOGIN_FAILED) {
        setStatus({ msg: "Sai tài khoản hoặc mật khẩu!", type: 'error' });
        alert("Sai tài khoản hoặc mật khẩu!");
      }
      else if (opcode === OPS.REGISTER_SUCCESS) {
        setStatus({ msg: "Đăng ký thành công! Đang chuyển trang...", type: 'success' });
        alert("Đăng ký thành công! Vui lòng đăng nhập.");
      }
      else if (opcode === OPS.REGISTER_FAILED) {
        setStatus({ msg: "Tài khoản đã tồn tại.", type: 'error' });
        alert("Tài khoản đã tồn tại.");
      }
    });

    return () => socket.off('server_to_client');
  }, [navigate]);

  const handleSubmit = (opcode, userInput, passInput) => {
    if (!userInput || !passInput) {
      setStatus({ msg: "Vui lòng nhập đầy đủ thông tin", type: 'error' });
      return;
    }

    const textData = `${userInput} ${passInput}`;
    const encoder = new TextEncoder();
    const stringBytes = encoder.encode(textData);

    const packet = new Uint8Array(1 + stringBytes.length);
    packet[0] = opcode;
    packet.set(stringBytes, 1);

    socket.emit("client_to_server", packet);
  };

  const handleLogin = (u, p) => {
    setUsername(u);
    setPassword(p);
    handleSubmit(OPS.LOGIN, u, p);
  };

  const handleRegister = (u, p) => {
    setUsername(u);
    setPassword(p);
    handleSubmit(OPS.REGISTER, u, p);
  };

  const handleLogout = () => {
    setUsername('');
    setPassword('');
    setStatus({ msg: '', type: '' });
    navigate('/');
  };

  const handleJoinGame = () => {
    navigate('/game');
  };

  return (
    <div className="min-h-screen bg-gray-900 font-sans text-white">
      <Routes>
        <Route path="/" element={<Login onLogin={handleLogin} onRegister={handleRegister} />} />
        <Route path="/home" element={<Lobby username={username} onLogout={handleLogout} onJoinGame={handleJoinGame} />} />
        <Route path="/game" element={<GameScreen />} />
      </Routes>
    </div>
  );
};

// --- APP ROOT ---

const App = () => {
  return (
    <BrowserRouter>
      <GameContent />
    </BrowserRouter>
  );
};

export default App;