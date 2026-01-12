import React, { useState, useEffect } from 'react';
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import { motion, AnimatePresence } from 'framer-motion';

// Hooks
import { useGameLogic } from './hooks/useGameLogic';
import { OPS } from './lib/ops';

// UI Components
import RoomPage from './components/RoomPage';
import LoginPage from './components/LoginPage';
import RegisterPage from './components/RegisterPage';
import HomePage from './components/HomePage';
import GameUI from './components/GameUI';
import AdminPage from './components/AdminPage';
import GameHistory from './components/GameHistory';
import Background from './components/Background';

const GameContent = () => {
  const { state, actions, socket } = useGameLogic();
  const {
    username, setUsername, password, setPassword,
    score, status,
    gameStatus, currentQuestion, timeLeft, gameResult, renderKey, currentGameScore,
    roomInfo, roomMembers, isHost,
    rooms, leaderboard,
    players,
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

  // Local state for lifelines (since logic might be shared in backend but we track UI usage here for now)
  const [usedLifelines, setUsedLifelines] = useState({
    1: false, // 50:50
    2: false, // Audience
    3: false, // Phone
    4: false  // Expert
  });

  // Reset lifelines when game starts/ends
  useEffect(() => {
    if (gameStatus === 'LOBBY') {
      setUsedLifelines({ 1: false, 2: false, 3: false, 4: false });
    }
  }, [gameStatus]);

  const handleUseLifeline = (lifelineId) => {
    if (usedLifelines[lifelineId]) return;

    // Optimistic UI update
    setUsedLifelines(prev => ({ ...prev, [lifelineId]: true }));

    // Send to server using OPS constant
    const encoder = new TextEncoder();
    const payload = encoder.encode(String(lifelineId));
    const fullPacket = new Uint8Array(1 + payload.length);
    fullPacket[0] = OPS.USE_HELP;
    fullPacket.set(payload, 1);

    socket.emit('client_to_server', fullPacket);
  };

  const commonProps = { username, setUsername, password, setPassword, onSubmit: handleSubmit, status };

  return (
    <div className="min-h-screen flex justify-center items-center font-sans p-4 relative overflow-hidden text-[#1A1A1A]">
      <Background />

      {/* Incoming Invite Notification */}
      <AnimatePresence>
        {incomingInvite && (
          <motion.div
            initial={{ y: -100, opacity: 0 }}
            animate={{ y: 0, opacity: 1 }}
            exit={{ y: -100, opacity: 0 }}
            className="fixed top-8 right-8 z-[100]"
          >
            <div className="bg-white/90 backdrop-blur-xl border border-[#1A1A1A] p-4 shadow-[8px_8px_0px_0px_rgba(26,26,26,1)] max-w-sm flex items-start gap-4">
              <div className="w-10 h-10 bg-ren-gold flex items-center justify-center text-white text-xl rounded-full">
                💌
              </div>
              <div className="flex-1">
                <h4 className="font-serif italic text-lg text-[#1A1A1A] mb-1">Invitation Received</h4>
                <p className="font-mono text-xs text-ren-gray mb-3">
                  <span className="font-bold text-[#1A1A1A]">{incomingInvite.hostName}</span> summons you to Sanctuary #{incomingInvite.roomId}
                </p>
                <div className="flex gap-2">
                  <button
                    onClick={handleAcceptInvite}
                    className="bg-[#1A1A1A] text-white px-3 py-1.5 text-[10px] font-bold uppercase tracking-widest hover:bg-ren-gold transition-colors"
                  >
                    Accept
                  </button>
                  <button
                    onClick={() => setIncomingInvite(null)}
                    className="text-[10px] font-bold uppercase tracking-widest px-3 py-1.5 hover:bg-gray-100 transition-colors"
                  >
                    Decline
                  </button>
                </div>
              </div>
            </div>
          </motion.div>
        )}
      </AnimatePresence>

      {/* Renaissance Help Overlay */}
      <AnimatePresence>
        {state.helpOverlay?.open && (
          <motion.div
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            className="fixed inset-0 z-[200] flex items-center justify-center bg-black/40 backdrop-blur-sm p-4"
            onClick={() => state.setHelpOverlay({ open: false, text: '' })} // Close on backdrop click
          >
            <motion.div
              initial={{ scale: 0.9, y: 20 }}
              animate={{ scale: 1, y: 0 }}
              exit={{ scale: 0.9, y: 20 }}
              onClick={(e) => e.stopPropagation()}
              className="bg-[#F9F8F6] border border-[#1A1A1A] p-8 max-w-md w-full shadow-[12px_12px_0px_0px_rgba(26,26,26,1)] relative overflow-hidden"
            >
              {/* Decorative Elements */}
              <div className="absolute top-0 left-0 w-20 h-20 bg-ren-gold/10 rounded-br-full" />
              <div className="absolute bottom-0 right-0 w-20 h-20 bg-ren-gold/10 rounded-tl-full" />

              <h3 className="font-serif italic text-2xl text-[#1A1A1A] mb-4 flex items-center gap-2">
                <span>🗝️</span> Guidance
              </h3>

              <div className="font-serif text-lg leading-relaxed text-ren-charcoal border-l-2 border-ren-gold pl-4 mb-8 italic bg-white/50 p-4 rounded-r-lg">
                {state.helpOverlay.text}
              </div>

              <div className="flex justify-end">
                <motion.button
                  whileHover={{ scale: 1.05 }}
                  whileTap={{ scale: 0.95 }}
                  onClick={() => state.setHelpOverlay({ open: false, text: '' })}
                  className="bg-[#1A1A1A] text-white px-6 py-2 text-xs font-bold uppercase tracking-widest hover:bg-ren-gold transition-colors shadow-lg"
                >
                  Acknowledged
                </motion.button>
              </div>
            </motion.div>
          </motion.div>
        )}
      </AnimatePresence>

      <Routes>
        <Route path="/" element={<LoginPage {...commonProps} />} />
        <Route path="/register" element={<RegisterPage {...commonProps} />} />
        <Route path="/home" element={
          <HomePage
            username={username}
            userId={state.userId}
            score={score}
            socket={socket}
            onLogout={handleLogout}
            onCreateRoom={handleCreateRoom}
            onJoinRoom={handleJoinRoom}
            rooms={rooms}
            leaderboard={leaderboard}
            onRequestLeaderboard={handleGetLeaderboard}
          />
        } />
        {/* Admin Page kept but not revamped to save focus */}
        <Route path="/admin" element={
          <AdminPage
            username={username}
            onLogout={handleLogout}
            allUsers={state.allUsers}
            onGetAllUsers={actions.handleGetAllUsers}
            onDeleteUser={actions.handleDeleteUser}
          />
        } />
        <Route path="/game-history" element={
          state.gameState === 'LOGIN' ? <Navigate to="/" replace /> : <GameHistory userId={state.userId} />
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
            <motion.div
              initial={{ scale: 0.9, opacity: 0 }}
              animate={{ scale: 1, opacity: 1 }}
              className="bg-white border border-[#1A1A1A] p-12 text-center max-w-lg shadow-[20px_20px_0px_0px_rgba(26,26,26,1)] relative overflow-hidden"
            >
              <div className="absolute top-0 left-0 w-full h-2 bg-gradient-to-r from-red-500 via-ren-gold to-blue-500" />

              <div className="mb-8">
                <h2 className="text-6xl font-serif italic text-[#1A1A1A] mb-4">
                  Concluded
                </h2>
                <p className="font-mono text-lg text-ren-gray mb-6 uppercase tracking-widest border-y border-gray-100 py-4">
                  {gameResult}
                </p>
              </div>

              <div className="grid grid-cols-2 gap-4">
                <button
                  onClick={handleReplay}
                  className="px-6 py-4 bg-[#1A1A1A] text-white font-serif italic text-lg hover:bg-ren-gold transition-colors shadow-lg"
                >
                  Replay Memory
                </button>
                <button
                  onClick={handleLeaveRoom}
                  className="px-6 py-4 bg-white border border-[#1A1A1A] text-[#1A1A1A] font-bold text-xs uppercase tracking-widest hover:bg-gray-50 transition-colors"
                >
                  Exit to Lobby
                </button>
              </div>
            </motion.div>
          ) : (
            <GameUI
              key={renderKey}
              question={currentQuestion}   // Mapped prop
              timer={timeLeft}             // Mapped prop
              score={currentGameScore}     // UPDATED: Mapped to session score
              onAnswer={handleAnswer}      // Mapped prop
              useLifeline={handleUseLifeline} // New handler
              onLeave={handleLeaveRoom}
              disableAnswer={false} // Can add logic if needed

              // Pass lifeline status
              is5050Used={usedLifelines[1]}
              isAudienceUsed={usedLifelines[2]}
              isPhoneUsed={usedLifelines[3]}
              isExpertUsed={usedLifelines[4]}

              room={roomInfo} // Pass room info for checks if needed
            />
          )
        } />
      </Routes>
    </div>
  );
};

const App = () => (
  <BrowserRouter>
    <GameContent />
  </BrowserRouter>
);

export default App;