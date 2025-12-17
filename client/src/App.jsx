import React, { useEffect, useState } from 'react';
import io from 'socket.io-client';
import { BrowserRouter, Routes, Route, useNavigate, Link } from 'react-router-dom';

// --- CONFIGURATION ---
const socket = io('http://localhost:4000');

const OPS = {
  LOGIN: 0x01,
  REGISTER: 0x02,
  LOGIN_SUCCESS: 0x03,
  LOGIN_FAILED: 0x04,
  ALREADY_LOGIN: 0x05,
  REGISTER_SUCCESS: 0x06,
  REGISTER_FAILED: 0x07,
  SERVER_FULL: 0x08,

  LOGOUT: 0x30,
};

// --- COMPONENTS ---

const LoginPage = ({ username, setUsername, password, setPassword, onSubmit, status }) => (
  <div className="bg-white p-8 rounded-2xl shadow-lg w-full max-w-sm">
    <h1 className="text-2xl font-bold text-center text-gray-900 mb-2">Đăng Nhập</h1>
    <p className="text-sm text-gray-500 text-center mb-6">Chào mừng bạn quay trở lại</p>

    {status.msg && (
      <div className={`mb-4 p-3 rounded-lg text-sm text-center ${status.type === 'success' ? 'bg-green-100 text-green-700 border border-green-300' : 'bg-red-100 text-red-700 border border-red-300'}`}>
        {status.msg}
      </div>
    )}

    <div className="mb-4">
      <label className="block text-sm font-medium text-gray-700 mb-1">Tên tài khoản</label>
      <input
        className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-400 outline-none transition"
        type="text"
        placeholder="Nhập username"
        value={username}
        onChange={(e) => setUsername(e.target.value)}
      />
    </div>

    <div className="mb-6">
      <label className="block text-sm font-medium text-gray-700 mb-1">Mật khẩu</label>
      <input
        className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-400 outline-none transition"
        type="password"
        placeholder="Nhập password"
        value={password}
        onChange={(e) => setPassword(e.target.value)}
      />
    </div>

    <button
      className="w-full bg-blue-600 hover:bg-blue-700 text-white py-2.5 rounded-lg font-semibold transition shadow-md"
      onClick={() => onSubmit(OPS.LOGIN)}
    >
      Đăng nhập
    </button>

    <p className="text-center text-sm text-gray-600 mt-4">
      Chưa có tài khoản?{" "}
      <Link className="text-blue-600 font-medium hover:underline" to="/register">
        Đăng ký ngay
      </Link>
    </p>
  </div>
);

const RegisterPage = ({ username, setUsername, password, setPassword, onSubmit, status }) => (
  <div className="bg-white p-8 rounded-2xl shadow-lg w-full max-w-sm">
    <h1 className="text-2xl font-bold text-center text-gray-900 mb-2">Tạo Tài Khoản</h1>
    <p className="text-sm text-gray-500 text-center mb-6">Tham gia đấu trường trí tuệ</p>

    {status.msg && (
      <div className={`mb-4 p-3 rounded-lg text-sm text-center ${status.type === 'success' ? 'bg-green-100 text-green-700 border border-green-300' : 'bg-red-100 text-red-700 border border-red-300'}`}>
        {status.msg}
      </div>
    )}

    <div className="mb-4">
      <label className="block text-sm font-medium text-gray-700 mb-1">Tên tài khoản</label>
      <input
        className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-400 outline-none transition"
        type="text"
        placeholder="Chọn username"
        value={username}
        onChange={(e) => setUsername(e.target.value)}
      />
    </div>

    <div className="mb-6">
      <label className="block text-sm font-medium text-gray-700 mb-1">Mật khẩu</label>
      <input
        className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-400 outline-none transition"
        type="password"
        placeholder="Tạo password"
        value={password}
        onChange={(e) => setPassword(e.target.value)}
      />
    </div>

    <button
      className="w-full bg-blue-600 hover:bg-blue-700 text-white py-2.5 rounded-lg font-semibold transition shadow-md"
      onClick={() => onSubmit(OPS.REGISTER)}
    >
      Đăng ký
    </button>

    <p className="text-center text-sm text-gray-600 mt-4">
      Đã có tài khoản?{" "}
      <Link className="text-blue-600 font-medium hover:underline" to="/">
        Đăng nhập
      </Link>
    </p>
  </div>
);

const HomePage = ({ username, onLogout }) => (
  <div className="bg-white p-8 rounded-2xl shadow-lg max-w-lg w-full text-center">
    <div className="w-20 h-20 bg-blue-600 text-white rounded-full mx-auto flex items-center justify-center text-3xl font-bold shadow-md">
      {username ? username.charAt(0).toUpperCase() : '?'}
    </div>

    <h1 className="text-2xl font-bold mt-4 text-gray-900">Xin chào, {username}!</h1>
    <p className="text-sm text-gray-500 mt-2">Bạn đã kết nối thành công tới Server C.</p>

    <div className="grid grid-cols-2 gap-4 mt-8">
      <button className="bg-green-500 hover:bg-green-600 text-white py-3 rounded-xl font-semibold transition shadow-sm">
        Vào Ngay
      </button>
      <button className="bg-yellow-500 hover:bg-yellow-600 text-white py-3 rounded-xl font-semibold transition shadow-sm">
        Tạo Phòng
      </button>
    </div>

    <button
      className="text-red-500 hover:text-red-700 hover:underline mt-6 font-medium text-sm transition"
      onClick={onLogout}
    >
      Đăng xuất
    </button>
  </div>
);

// --- MAIN LOGIC CONTAINER ---

const GameContent = () => {
  const navigate = useNavigate();
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
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
      }
      else if (opcode === OPS.REGISTER_SUCCESS) {
        setStatus({ msg: "Đăng ký thành công! Đang chuyển trang...", type: 'success' });
        setTimeout(() => {
          setStatus({ msg: '', type: '' });
          navigate('/');
        }, 1500);
      }
      else if (opcode === OPS.REGISTER_FAILED) {
        setStatus({ msg: "Tài khoản đã tồn tại.", type: 'error' });
      }
      else if (opcode === OPS.ALREADY_LOGIN) {
        setStatus({ msg: "Bạn đã đăng nhập rồi!", type: 'error' });
      }
      else if (opcode === OPS.SERVER_FULL) {
        setStatus({ msg: "Server đã đầy!", type: 'error' });
      }
    });

    return () => socket.off('server_to_client');
  }, [navigate]);

  const handleSubmit = (opcode) => {
    if (!username || !password) {
      setStatus({ msg: "Vui lòng nhập đầy đủ thông tin", type: 'error' });
      return;
    }

    const textData = `${username} ${password}`;
    const encoder = new TextEncoder();
    const stringBytes = encoder.encode(textData);

    const packet = new Uint8Array(1 + stringBytes.length);
    packet[0] = opcode;
    packet.set(stringBytes, 1);

    socket.emit("client_to_server", packet);
  };

  const handleLogout = () => {
    const packet = new Uint8Array(1);
    packet[0] = OPS.LOGOUT;
    socket.emit("client_to_server", packet);

    setUsername('');
    setPassword('');
    setStatus({ msg: '', type: '' });
    navigate('/');
  };

  const commonProps = { username, setUsername, password, setPassword, onSubmit: handleSubmit, status };

  return (
    <div className="min-h-screen flex justify-center items-center bg-gray-100 font-sans">
      <Routes>
        <Route path="/" element={<LoginPage {...commonProps} />} />
        <Route path="/register" element={<RegisterPage {...commonProps} />} />
        <Route path="/home" element={<HomePage username={username} onLogout={handleLogout} />} />
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