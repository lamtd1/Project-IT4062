import React, { useState } from 'react';
import { motion, AnimatePresence } from 'framer-motion';

// Sub-components can be imported or defined here for clean single-file structure if preferred
// But we'll assume they exist and wrap them or restyle them.
// Actually, HomePage orchestrates RoomListPage, CreateRoomPanel, Leaderboard.
// We will revamp HomePage layout to be a 'Dashboard'

import RoomListPage from './RoomListPage';
import CreateRoomPanel from './CreateRoomPanel';
import Leaderboard from './Leaderboard';
import GameHistory from './GameHistory';

const HomePage = ({ username, userId, score, socket, onLogout, onCreateRoom, onJoinRoom, rooms, leaderboard, onRequestLeaderboard }) => {
  const [showHistory, setShowHistory] = useState(false);

  return (
    <motion.div
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      className="w-full max-w-7xl mx-auto p-4 md:p-8 h-screen flex flex-col relative"
    >
      {/* Header / Top Bar */}
      <header className="flex justify-between items-end mb-12 border-b border-[#1A1A1A] pb-6">
        <div>
          <h1 className="text-6xl font-serif italic text-[#1A1A1A] leading-tight tracking-tighter">
            Sanctuary
          </h1>
          <div className="flex items-center gap-3 mt-4 text-xs font-mono uppercase tracking-widest bg-ren-charcoal text-white px-4 py-2 rounded-full shadow-md">
            <span className="w-2 h-2 bg-green-500 rounded-full animate-pulse" />
            <span className="opacity-80">Connected: {username}</span>
            <span className="w-px h-3 bg-white/20 mx-2" />
            <span className="text-ren-gold font-bold">Score: {score}</span>
          </div>
        </div>
        <div className="flex gap-6 items-center">
          <button
            onClick={() => setShowHistory(true)}
            className="group flex items-center gap-2 text-xs font-bold uppercase tracking-widest text-[#1A1A1A] hover:text-ren-gold transition-colors"
          >
            Archives
            <span className="block h-[1px] w-8 bg-[#1A1A1A] group-hover:bg-ren-gold transition-colors" />
          </button>
          <button
            onClick={onLogout}
            className="group flex items-center gap-2 text-xs font-bold uppercase tracking-widest hover:text-red-600 transition-colors"
          >
            Disconnect
            <span className="block h-[1px] w-8 bg-[#1A1A1A] group-hover:bg-red-600 transition-colors" />
          </button>
        </div>
      </header>

      {/* Main Content Grid */}
      <div className="flex-1 grid grid-cols-12 gap-8 min-h-0">

        {/* Left Column: Create & Leaderboard (4 cols) */}
        <div className="col-span-12 lg:col-span-4 flex flex-col gap-8 overflow-y-auto custom-scrollbar pr-2">
          <section>
            <CreateRoomPanel onCreate={onCreateRoom} />
          </section>

          <section className="flex-1 min-h-[300px] border border-[#1A1A1A] bg-[#F9F8F6]/90 backdrop-blur-md p-6 shadow-[4px_4px_0px_0px_rgba(26,26,26,0.5)] relative overflow-hidden">
            <div className="absolute top-0 right-0 p-2 opacity-10">
              <span className="text-6xl">🏆</span>
            </div>
            <h3 className="text-2xl font-serif italic mb-6 border-b border-gray-200 pb-2">Hall of Fame</h3>
            <Leaderboard
              leaderboard={leaderboard}
              onRequest={onRequestLeaderboard}
              currentUserId={userId}
            />
          </section>
        </div>

        {/* Right Column: Room List (8 cols) */}
        <div className="col-span-12 lg:col-span-8 flex flex-col h-full min-h-0">
          <div className="flex-1 border border-[#1A1A1A] bg-white/50 backdrop-blur-md p-8 shadow-[8px_8px_0px_0px_rgba(26,26,26,1)] flex flex-col overflow-hidden relative">
            {/* Decor */}
            <div className="absolute -top-10 -right-10 w-40 h-40 bg-ren-gold/10 rounded-full blur-3xl pointer-events-none" />

            <div className="flex justify-between items-center mb-8 z-10">
              <h2 className="text-4xl font-serif italic text-ren-charcoal">Active Rituals</h2>
              <div className="text-xs font-mono bg-ren-charcoal text-white px-3 py-1 rounded-full">
                {rooms.length} LIVE
              </div>
            </div>

            <div className="flex-1 overflow-y-auto pr-2 -mr-2 custom-scrollbar z-10">
              <RoomListPage rooms={rooms} onJoin={onJoinRoom} />
            </div>
          </div>
        </div>
      </div>

      {/* GAME HISTORY MODAL OVERLAY */}
      <AnimatePresence>
        {showHistory && (
          <motion.div
            initial={{ opacity: 0, y: 50 }}
            animate={{ opacity: 1, y: 0 }}
            exit={{ opacity: 0, y: 50 }}
            className="absolute inset-0 z-50 bg-[#F9F8F6] flex flex-col"
          >
            <button
              onClick={() => setShowHistory(false)}
              className="absolute top-8 right-8 z-50 flex items-center gap-2 group"
            >
              <span className="text-xs font-bold uppercase tracking-widest group-hover:text-ren-gold transition-colors">Close Archives</span>
              <div className="w-8 h-8 rounded-full border border-[#1A1A1A] flex items-center justify-center group-hover:bg-[#1A1A1A] group-hover:text-white transition-all">
                ✕
              </div>
            </button>

            {/* Render GameHistory fully inside */}
            <div className="flex-1 overflow-y-auto custom-scrollbar">
              <GameHistory userId={userId} />
            </div>
          </motion.div>
        )}
      </AnimatePresence>

    </motion.div>
  );
};

export default HomePage;
