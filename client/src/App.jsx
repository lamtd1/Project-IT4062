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

  ROOM_CREATE: 0x10,
  ROOM_JOIN: 0x11,

  LOGOUT: 0x30,
  LEAVE_ROOM: 0x31,

  GET_ROOMS: 0x27,
  ROOM_LIST: 0x28,
};

// --- COMPONENTS ---

const RoomListPage = ({ rooms, onJoin, onCreate, onBack }) => (
  <div className="bg-white p-6 rounded-2xl shadow-lg w-full max-w-2xl">
    <div className="flex justify-between items-center mb-6">
      <h1 className="text-2xl font-bold text-gray-800">Danh sách phòng</h1>
      <div className="space-x-2">
        <button onClick={onCreate} className="px-4 py-2 bg-yellow-500 hover:bg-yellow-600 text-white rounded-lg font-medium text-sm">
          + Tạo Phòng
        </button>
        <button onClick={onBack} className="px-4 py-2 bg-gray-200 hover:bg-gray-300 text-gray-700 rounded-lg font-medium text-sm">
          Quay lại
        </button>
      </div>
    </div>

    <div className="overflow-y-auto max-h-[400px]">
      {rooms.length === 0 ? (
        <p className="text-center text-gray-500 py-8">Chưa có phòng nào. Hãy tạo phòng mới!</p>
      ) : (
        <div className="grid gap-3">
          {rooms.map((room) => (
            <div key={room.id} className="flex justify-between items-center p-4 border border-gray-100 rounded-xl hover:bg-blue-50 transition bg-gray-50">
              <div>
                <h3 className="font-bold text-gray-800">{room.name}</h3>
                <p className="text-xs text-gray-500">ID: {room.id} • {room.status === '0' ? 'Minh bach' : 'Dang choi'}</p>
              </div>
              <div className="flex items-center gap-4">
                <span className={`text-sm font-semibold ${room.count >= 4 ? 'text-red-500' : 'text-green-500'}`}>
                  {room.count}/4
                </span>
                <button
                  onClick={() => onJoin(room.id)}
                  disabled={room.count >= 4 || room.status !== '0'}
                  className={`px-4 py-2 rounded-lg font-medium text-sm text-white transition shadow-sm
                                        ${(room.count >= 4 || room.status !== '0') ? 'bg-gray-300 cursor-not-allowed' : 'bg-blue-600 hover:bg-blue-700'}
                                    `}
                >
                  {room.count >= 4 ? 'Đầy' : 'Vào Ngay'}
                </button>
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  </div>
);

const RoomPage = ({ roomInfo, onLeave }) => (
  <div className="bg-white p-8 rounded-2xl shadow-lg max-w-lg w-full text-center">
    <h1 className="text-2xl font-bold text-blue-600 mb-2">Phòng chờ</h1>
    <p className="text-lg font-semibold text-gray-800">
      {roomInfo.name || `Room #${roomInfo.id}`}
    </p>
    <div className="mt-6 p-4 bg-gray-50 rounded-xl border border-gray-200">
      <p className="text-gray-500 text-sm mb-2">Trạng thái</p>
      <div className="flex items-center justify-center space-x-2">
        <div className="w-3 h-3 bg-green-500 rounded-full animate-pulse"></div>
        <span className="font-medium text-gray-700">Đang đợi người chơi...</span>
      </div>
    </div>

    <button
      onClick={onLeave}
      className="mt-8 px-6 py-2 bg-red-500 hover:bg-red-600 text-white rounded-lg font-medium transition"
    >
      Rời phòng
    </button>
  </div>
);

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

const HomePage = ({ username, onLogout, onPlayNow }) => (
  <div className="bg-white p-8 rounded-2xl shadow-lg max-w-lg w-full text-center">
    <div className="w-20 h-20 bg-blue-600 text-white rounded-full mx-auto flex items-center justify-center text-3xl font-bold shadow-md">
      {username ? username.charAt(0).toUpperCase() : '?'}
    </div>

    <h1 className="text-2xl font-bold mt-4 text-gray-900">Xin chào, {username}!</h1>
    <p className="text-sm text-gray-500 mt-2">Bạn đã kết nối thành công tới Server C.</p>

    <div className="grid grid-cols-1 gap-4 mt-8">
      <button
        onClick={onPlayNow}
        className="bg-green-500 hover:bg-green-600 text-white py-4 rounded-xl font-bold text-lg transition shadow-md transform hover:scale-105"
      >
        Vào Chơi Ngay
      </button>
    </div>

    <button
      className="text-red-500 hover:text-red-700 hover:underline mt-8 font-medium text-sm transition"
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
  const [roomInfo, setRoomInfo] = useState({ id: null, name: '' });
  const [rooms, setRooms] = useState([]);

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
      // --- ROOM RESPONSE ---
      else if (opcode === OPS.ROOM_CREATE) {
        const success = view[1];
        if (success) {
          setStatus({ msg: "Tạo phòng thành công!", type: 'success' });
          navigate('/room');
        } else {
          setStatus({ msg: "Tạo phòng thất bại!", type: 'error' });
        }
      }
      else if (opcode === OPS.ROOM_JOIN) {
        const success = view[1];
        if (success) {
          setStatus({ msg: "Vào phòng thành công!", type: 'success' });
          navigate('/room');
        } else {
          setStatus({ msg: "Không tìm thấy phòng hoặc phòng đầy!", type: 'error' });
        }
      }
      else if (opcode === OPS.ROOM_LIST) {
        // Payload: "id:name:count:status,id2..."
        const textDecoder = new TextDecoder();
        const listStr = textDecoder.decode(view.slice(1));

        if (!listStr) {
          setRooms([]);
        } else {
          const items = listStr.split(',');
          const parsedRooms = items.map(item => {
            const [id, name, count, status] = item.split(':');
            return { id, name, count, status };
          });
          setRooms(parsedRooms);
        }
        navigate('/rooms');
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

  const handleGetRooms = () => {
    const packet = new Uint8Array(1);
    packet[0] = OPS.GET_ROOMS;
    socket.emit("client_to_server", packet);
  };

  const handleCreateRoom = () => {
    const roomName = prompt("Nhập tên phòng muốn tạo:");
    if (!roomName) return;

    const encoder = new TextEncoder();
    const stringBytes = encoder.encode(roomName);
    const packet = new Uint8Array(1 + stringBytes.length);
    packet[0] = OPS.ROOM_CREATE;
    packet.set(stringBytes, 1);
    socket.emit("client_to_server", packet);

    setRoomInfo({ id: 0, name: roomName });
  };

  const handleJoinRoom = (roomId) => {
    const encoder = new TextEncoder();
    // roomId is string from parsed list
    const stringBytes = encoder.encode(roomId);
    const packet = new Uint8Array(1 + stringBytes.length);
    packet[0] = OPS.ROOM_JOIN;
    packet.set(stringBytes, 1);
    socket.emit("client_to_server", packet);

    // Find room name to display
    const r = rooms.find(rm => rm.id === roomId);
    setRoomInfo({ id: roomId, name: r ? r.name : '' });
  };

  const handleLeaveRoom = () => {
    const packet = new Uint8Array(1);
    packet[0] = OPS.LEAVE_ROOM;
    socket.emit("client_to_server", packet);
    navigate('/home');
  };

  const commonProps = { username, setUsername, password, setPassword, onSubmit: handleSubmit, status };

  return (
    <div className="min-h-screen flex justify-center items-center bg-gray-100 font-sans">
      <Routes>
        <Route path="/" element={<LoginPage {...commonProps} />} />
        <Route path="/register" element={<RegisterPage {...commonProps} />} />
        <Route path="/home" element={
          <HomePage
            username={username}
            onLogout={handleLogout}
            onPlayNow={handleGetRooms}
          />
        } />
        <Route path="/rooms" element={
          <RoomListPage
            rooms={rooms}
            onJoin={handleJoinRoom}
            onCreate={handleCreateRoom}
            onBack={() => navigate('/home')}
          />
        } />
        <Route path="/room" element={<RoomPage roomInfo={roomInfo} onLeave={handleLeaveRoom} />} />
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