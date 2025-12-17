import React from 'react';
import { BrowserRouter, Routes, Route } from 'react-router-dom';

// Import components
import RoomListPage from './components/RoomListPage'; // Unused in previous App.jsx? Kept for safety if standard imports
import CreateRoomPanel from './components/CreateRoomPanel'; // Unused?
import Leaderboard from './components/Leaderboard'; // Unused? 
// Actually they pass these *down* to HomePage.

// Hooks
import { useGameLogic } from './hooks/useGameLogic';

// UI Components
import RoomPage from './components/RoomPage';
import LoginPage from './components/LoginPage';
import RegisterPage from './components/RegisterPage';
import HomePage from './components/HomePage';
import GameUI from './components/GameUI';
import { Button } from "./components/ui/button";

const GameContent = () => {
  const { state, actions, socket } = useGameLogic();
  const {
    username, setUsername, password, setPassword,
    score, status,
    gameStatus, currentQuestion, timeLeft, gameResult, renderKey,
    roomInfo, roomMembers, isHost,
    rooms, leaderboard,
    idleUsers, incomingInvite, setIncomingInvite
  } = state;

  const {
    handleSubmit,
    handleCreateRoom,
    handleJoinRoom,
    handleLeaveRoom,
    handleLogout,
    handleGetLeaderboard,
    handleStartGame,
    handleAnswer,
    handleGetIdleUsers,
    handleSendInvite,
    handleAcceptInvite
  } = actions;

  const commonProps = { username, setUsername, password, setPassword, onSubmit: handleSubmit, status };

  return (
    <div className="min-h-screen flex justify-center items-center bg-blue-50 font-sans p-4 relative">
      {/* Incoming Invite Notification */}
      {incomingInvite && (
        <div className="fixed top-4 right-4 z-50 animate-in fade-in slide-in-from-top-5">
          <div className="bg-white border border-indigo-200 shadow-xl rounded-lg p-4 flex items-center gap-4 max-w-sm">
            <div className="h-10 w-10 bg-indigo-100 rounded-full flex items-center justify-center text-indigo-600 font-bold">
              💌
            </div>
            <div className="flex-1">
              <h4 className="font-bold text-gray-900 text-sm">{incomingInvite.hostName} invites you!</h4>
              <p className="text-xs text-gray-500">Join room #{incomingInvite.roomId}?</p>
            </div>
            <div className="flex gap-2">
              <Button size="sm" variant="ghost" onClick={() => setIncomingInvite(null)} className="h-8 w-8 p-0 text-gray-400 hover:text-gray-600">✕</Button>
              <Button size="sm" onClick={handleAcceptInvite} className="h-8 bg-indigo-600 hover:bg-indigo-700 text-white">Join</Button>
            </div>
          </div>
        </div>
      )}

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

              idleUsers={idleUsers}
              onGetIdleUsers={handleGetIdleUsers}
              onSendInvite={handleSendInvite}
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