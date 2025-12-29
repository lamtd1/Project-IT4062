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
import AdminPage from './components/AdminPage';
import { Button } from "./components/ui/button";

const GameContent = () => {
  const { state, actions, socket } = useGameLogic();
  const {
    username, setUsername, password, setPassword,
    score, status,
    gameStatus, setGameStatus, currentQuestion, timeLeft, gameResult, renderKey,
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
    handleAcceptInvite,
    handleReplay
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
        <Route path="/admin" element={
          <AdminPage
            username={username}
            onLogout={handleLogout}
            allUsers={state.allUsers}
            onGetAllUsers={actions.handleGetAllUsers}
            onDeleteUser={actions.handleDeleteUser}
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
            <div className="bg-white p-8 rounded-2xl shadow-2xl text-center max-w-lg border-2 border-zinc-100">
              <div className="mb-6">
                <div className="w-20 h-20 bg-gradient-to-br from-indigo-500 to-purple-600 rounded-full flex items-center justify-center mx-auto mb-4 shadow-lg">
                  <span className="text-4xl">🏆</span>
                </div>
                <h2 className="text-4xl font-bold text-zinc-900 mb-2">Game Kết Thúc!</h2>
                <p className="text-lg text-zinc-600 mb-4">{gameResult}</p>
              </div>

              <div className="flex flex-col sm:flex-row gap-4 justify-center mt-8">
                <button
                  onClick={handleReplay}
                  className="group relative px-8 py-3 bg-gradient-to-r from-indigo-600 to-purple-600 text-white rounded-xl font-bold shadow-lg hover:shadow-xl transform hover:scale-105 transition-all duration-200 overflow-hidden"
                >
                  <span className="relative z-10 flex items-center justify-center gap-2">
                    🔄 Chơi lại
                  </span>
                  <div className="absolute inset-0 bg-gradient-to-r from-purple-600 to-indigo-600 opacity-0 group-hover:opacity-100 transition-opacity"></div>
                </button>

                <button
                  onClick={handleLeaveRoom}
                  className="px-8 py-3 bg-zinc-100 hover:bg-zinc-200 text-zinc-700 rounded-xl font-bold shadow-md hover:shadow-lg transform hover:scale-105 transition-all duration-200 border-2 border-zinc-200"
                >
                  <span className="flex items-center justify-center gap-2">
                    🏠 Về Sảnh
                  </span>
                </button>
              </div>
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