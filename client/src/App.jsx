import React, { useEffect, useState, useCallback } from 'react';
import io from 'socket.io-client';
import { BrowserRouter, Routes, Route, useNavigate } from 'react-router-dom';

// Import components
import Leaderboard from './components/Leaderboard';
import RoomListPage from './components/RoomListPage';
import CreateRoomPanel from './components/CreateRoomPanel';
import RoomPage from './components/RoomPage';
import LoginPage from './components/LoginPage';
import RegisterPage from './components/RegisterPage';
import HomePage from './components/HomePage';
import GameUI from './components/GameUI';

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
  GET_ROOM_DETAIL: 0x29,
  ROOM_DETAIL: 0x2A,

  GET_LEADERBOARD: 0x45,
  LEADERBOARD_LIST: 0x46,
};

// --- MAIN LOGIC CONTAINER ---

const GameContent = () => {
  const navigate = useNavigate();
  const [username, setUsername] = useState('');
  const [userId, setUserId] = useState(null);
  const [score, setScore] = useState(0);
  const [password, setPassword] = useState('');
  const [status, setStatus] = useState({ msg: '', type: '' });

  // Game State
  const [gameStatus, setGameStatus] = useState('LOBBY'); // LOBBY, PLAYING, FINISHED
  const [currentQuestion, setCurrentQuestion] = useState(null);
  const [timeLeft, setTimeLeft] = useState(0);
  const [gameResult, setGameResult] = useState("");
  const [renderKey, setRenderKey] = useState(0); // Force re-render counter

  const [roomInfo, setRoomInfo] = useState({ id: null, name: '' });
  const [roomMembers, setRoomMembers] = useState([]);
  const [isHost, setIsHost] = useState(false);

  const [rooms, setRooms] = useState([]);
  const [leaderboard, setLeaderboard] = useState([]);

  useEffect(() => {
    // Poll rooms and leaderboard periodically if on home screen
    const interval = setInterval(() => {
      if (window.location.pathname === '/home') {
        const p1 = new Uint8Array(1); p1[0] = OPS.GET_ROOMS;
        socket.emit("client_to_server", p1);
      }
      else if (window.location.pathname === '/room') {
        // Poll room details
        if (roomInfo.id !== null && roomInfo.id !== undefined) { // Check valid ID (0 is valid)
          const textData = `${roomInfo.id}`;
          const encoder = new TextEncoder();
          const stringBytes = encoder.encode(textData);
          const packet = new Uint8Array(1 + stringBytes.length);
          packet[0] = OPS.GET_ROOM_DETAIL;
          packet.set(stringBytes, 1);
          socket.emit("client_to_server", packet);
        }
      }
    }, 5000); // Poll every 5s (reduced freq from 2s)

    socket.on('server_to_client', (data) => {
      const view = new Uint8Array(data);
      const opcode = view[0];
      const textDecoder = new TextDecoder();

      if (opcode === OPS.LOGIN_SUCCESS) {
        // Payload: "ID:Score"
        const payload = textDecoder.decode(view.slice(1));
        const [idStr, scoreStr] = payload.split(':');

        setUserId(parseInt(idStr));
        setScore(parseInt(scoreStr || '0'));

        setStatus({ msg: "Đăng nhập thành công!", type: 'success' });
        setTimeout(() => navigate('/home'), 500);
      }
      else if (opcode === OPS.LOGIN_FAILED) {
        setStatus({ msg: "Sai tài khoản hoặc mật khẩu!", type: 'error' });
      }
      else if (opcode === OPS.REGISTER_SUCCESS) {
        setStatus({ msg: "Đăng ký thành công!", type: 'success' });
        setTimeout(() => { setStatus({ msg: '', type: '' }); navigate('/'); }, 1500);
      }
      else if (opcode === OPS.REGISTER_FAILED) setStatus({ msg: "Tài khoản đã tồn tại.", type: 'error' });
      else if (opcode === OPS.ALREADY_LOGIN) setStatus({ msg: "Bạn đã đăng nhập rồi!", type: 'error' });
      else if (opcode === OPS.SERVER_FULL) setStatus({ msg: "Server đã đầy!", type: 'error' });

      // --- ROOM RESPONSE ---
      else if (opcode === OPS.ROOM_CREATE) {
        const success = view[1];
        if (success) {
          const rId = view[2];
          setRoomInfo(prev => ({ ...prev, id: rId }));
          setIsHost(true);
          setStatus({ msg: `Tạo phòng ${rId} thành công!`, type: 'success' });
          navigate('/room');
        } else setStatus({ msg: "Tạo phòng thất bại!", type: 'error' });
      }
      else if (opcode === OPS.ROOM_JOIN) {
        const success = view[1];
        if (success) {
          setIsHost(false);
          navigate('/room');
        } else setStatus({ msg: "Vào phòng thất bại!", type: 'error' });
      }
      else if (opcode === OPS.LEAVE_ROOM) {
        // Server Kick / Host Left
        alert("Phòng đã bị hủy do chủ phòng thoát!");
        navigate('/home');
        setRoomMembers([]);
        setRoomInfo({});
        setGameStatus('LOBBY');
      }

      // --- GAME RESPONSES ---
      else if (opcode === 0x21) { // MSG_QUESTION
        const payload = textDecoder.decode(view.slice(1));
        // Format: "Level|Content|A|B|C|D|Duration"
        const parts = payload.split('|');
        if (parts.length >= 7) {
          const [lvl, content, a, b, c, d, dur] = parts;
          // Create a completely new object to force React re-render
          const newQuestion = {
            id: parseInt(lvl),
            content: content,
            answers: [a, b, c, d]
          };
          console.log(`[GAME] Question ${lvl}: ${content}`);
          setCurrentQuestion(newQuestion);
          setTimeLeft(parseInt(dur));
          setGameStatus('PLAYING');
          setRenderKey(prev => prev + 1); // Force re-render
        }
      }
      else if (opcode === 0x23) { // MSG_ANSWER_RESULT
        const payload = textDecoder.decode(view.slice(1));
        // Don't use alert - it blocks the UI and prevents next question from showing
        console.log('[ANSWER RESULT]', payload);
        // Could add a toast notification here instead
      }
      else if (opcode === 0x26) { // MSG_GAME_END
        const payload = textDecoder.decode(view.slice(1));
        setGameStatus('FINISHED');
        setGameResult(payload);
      }
      else if (opcode === OPS.ROOM_LIST) {
        const listStr = textDecoder.decode(view.slice(1));
        if (!listStr) setRooms([]);
        else {
          setRooms(listStr.split(',').map(item => {
            const [id, name, count, rStatus] = item.split(':');
            return { id, name, count, status: rStatus };
          }));
        }
      }
      else if (opcode === OPS.LEADERBOARD_LIST) {
        const listStr = textDecoder.decode(view.slice(1));
        if (!listStr) setLeaderboard([]);
        else {
          setLeaderboard(listStr.split(',').map(item => {
            const [name, sc] = item.split(':');
            return { name, score: sc };
          }));
        }
      }
      else if (opcode === OPS.ROOM_DETAIL) {
        const listStr = textDecoder.decode(view.slice(1));
        if (!listStr) setRoomMembers([]);
        else {
          const mems = listStr.split(',').map(item => {
            const [isH, name, sc] = item.split(':'); // "host_flag:username:score"
            return { isHost: isH === '1', username: name, score: sc };
          });
          setRoomMembers(mems);

          // Verify host logic locally ?
          // If I am in members list and isHost=1 -> I am host.
        }
      }
    });

    return () => { socket.off('server_to_client'); clearInterval(interval); };
  }, [navigate, roomInfo.id]);

  const handleSubmit = (opcode) => {
    if (!username || !password) return setStatus({ msg: "Nhập đủ thông tin", type: 'error' });
    const text = `${username} ${password}`;
    const encoder = new TextEncoder();
    const bytes = encoder.encode(text);
    const packet = new Uint8Array(1 + bytes.length);
    packet[0] = opcode;
    packet.set(bytes, 1);
    socket.emit("client_to_server", packet);
  };

  const handleCreateRoom = (roomName) => {
    const encoder = new TextEncoder();
    const bytes = encoder.encode(roomName);
    const packet = new Uint8Array(1 + bytes.length);
    packet[0] = OPS.ROOM_CREATE;
    packet.set(bytes, 1);
    socket.emit("client_to_server", packet);
    setRoomInfo({ id: 0, name: roomName });
    setGameStatus('LOBBY'); // Reset game state
  };

  const handleJoinRoom = (roomId) => {
    const packet = new Uint8Array(1 + roomId.length);
    packet[0] = OPS.ROOM_JOIN;
    const encoder = new TextEncoder();
    packet.set(encoder.encode(roomId), 1);
    socket.emit("client_to_server", packet);

    const r = rooms.find(rm => rm.id === roomId);
    setRoomInfo({ id: roomId, name: r ? r.name : `Phòng ${roomId}` });
    setGameStatus('LOBBY'); // Reset game state
  };

  const handleLeaveRoom = () => {
    const packet = new Uint8Array(1);
    packet[0] = OPS.LEAVE_ROOM;
    socket.emit("client_to_server", packet);
    navigate('/home');
    setRoomMembers([]);
    setGameStatus('LOBBY'); // Reset game state
  };

  const handleLogout = () => {
    const packet = new Uint8Array(1);
    packet[0] = OPS.LOGOUT;
    socket.emit("client_to_server", packet);
    // Reset all state
    setUsername('');
    setUserId(null);
    setScore(0);
    setGameStatus('LOBBY');
    setRoomInfo({ id: null, name: '' });
    setRoomMembers([]);
    navigate('/');
  };

  const handleGetLeaderboard = () => {
    if (!socket) return;
    const packet = new Uint8Array(1);
    packet[0] = 0x45; // MSG_GET_LEADERBOARD
    socket.emit("client_to_server", packet);
  };

  const handleStartGame = () => {
    const packet = new Uint8Array(1);
    packet[0] = 0x20; // MSG_GAME_START
    socket.emit("client_to_server", packet);
    console.log("Sent GAME_START to server");
  };

  const handleAnswer = (ansChar) => {
    const encoder = new TextEncoder();
    const bytes = encoder.encode(ansChar);
    const packet = new Uint8Array(1 + bytes.length);
    packet[0] = 0x22; // MSG_ANSWER (0x22)
    packet.set(bytes, 1);
    socket.emit("client_to_server", packet);
  };

  const commonProps = { username, setUsername, password, setPassword, onSubmit: handleSubmit, status };

  return (
    <div className="min-h-screen flex justify-center items-center bg-blue-50 font-sans p-4">
      <Routes>
        <Route path="/" element={<LoginPage {...commonProps} />} />
        <Route path="/register" element={<RegisterPage {...commonProps} />} />
        <Route path="/home" element={
          <HomePage
            username={username}
            score={score}
            onLogout={handleLogout}
            onCreateRoom={handleCreateRoom}
            onJoinRoom={handleJoinRoom}
            rooms={rooms}
            leaderboard={leaderboard}
            onRequestLeaderboard={handleGetLeaderboard}
          />
        } />
        <Route path="/room" element={
          gameStatus === 'LOBBY' ? (
            <RoomPage
              roomInfo={roomInfo}
              members={roomMembers}
              isHost={isHost}
              onLeave={handleLeaveRoom}
              onStart={handleStartGame}
            />
          ) : gameStatus === 'FINISHED' ? (
            <div className="bg-white p-8 rounded-xl shadow-xl text-center">
              <h2 className="text-3xl font-bold text-gray-800 mb-4">Kết Thúc!</h2>
              <p className="text-xl text-blue-600 mb-6">{gameResult}</p>
              <button onClick={handleLeaveRoom} className="bg-gray-500 text-white px-6 py-2 rounded-lg">Thoát về Sảnh</button>
            </div>
          ) : (
            <GameUI
              key={renderKey} // Force re-render using counter
              currentQuestion={currentQuestion}
              timeLeft={timeLeft}
              handleAnswer={handleAnswer}
              socket={socket}
            />
          )
        } />
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