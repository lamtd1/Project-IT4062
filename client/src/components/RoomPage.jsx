import React, { useState } from 'react';
import { motion, AnimatePresence } from 'framer-motion';

const RoomPage = ({ roomInfo, members, isHost, onLeave, onStart, idleUsers = [], onGetIdleUsers, onSendInvite }) => {
    const [showInvite, setShowInvite] = useState(false);

    const isMode0 = roomInfo.name && roomInfo.name.includes(':0');
    const maxPlayers = isMode0 ? 1 : 4;

    const handleOpenInvite = () => {
        if (onGetIdleUsers) onGetIdleUsers();
        setShowInvite(true);
    };

    const handleInviteClick = (user) => {
        if (onSendInvite) onSendInvite(user);
    };

    return (
        <React.Fragment>
            {/* Invitation Modal */}
            <AnimatePresence>
                {showInvite && (
                    <motion.div
                        initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                        className="fixed inset-0 bg-black/60 backdrop-blur-sm z-50 flex items-center justify-center p-4"
                    >
                        <motion.div
                            initial={{ scale: 0.95, y: 20 }} animate={{ scale: 1, y: 0 }} exit={{ scale: 0.95, y: 20 }}
                            className="w-full max-w-md bg-white/90 backdrop-blur-xl shadow-2xl border border-white/50 rounded-2xl overflow-hidden"
                        >
                            <div className="flex flex-row items-center justify-between p-6 border-b border-gray-100">
                                <h3 className="text-xl font-serif italic text-ren-charcoal">Summon Allies</h3>
                                <button onClick={() => setShowInvite(false)} className="px-3 py-1 hover:bg-gray-100 rounded-full transition">✕</button>
                            </div>
                            <div className="space-y-4 px-6 pb-6 pt-4">
                                <div className="flex justify-between items-center bg-gray-50/50 p-2 rounded-lg text-xs text-ren-gray border border-gray-100">
                                    <span className="font-bold uppercase tracking-wider">Available Souls: {idleUsers.length}</span>
                                    <button onClick={onGetIdleUsers} className="text-ren-gold hover:text-ren-charcoal transition-colors font-bold">Refresh</button>
                                </div>
                                <div className="max-h-[300px] overflow-y-auto space-y-2 pr-2 custom-scrollbar">
                                    {idleUsers.length === 0 ? (
                                        <div className="text-center py-8 text-ren-gray italic">The void is empty...</div>
                                    ) : (
                                        idleUsers.map((u, idx) => (
                                            <motion.div
                                                key={idx}
                                                initial={{ opacity: 0, x: -10 }} animate={{ opacity: 1, x: 0 }} delay={idx * 0.05}
                                                className="flex justify-between items-center p-3 rounded-xl border border-gray-100 hover:border-ren-gold/30 hover:bg-white transition-all group"
                                            >
                                                <div className="flex items-center gap-3">
                                                    <div className="w-8 h-8 rounded-full bg-ren-charcoal text-white flex items-center justify-center text-xs font-serif italic">
                                                        {u.charAt(0).toUpperCase()}
                                                    </div>
                                                    <span className="font-medium text-ren-charcoal">{u}</span>
                                                </div>
                                                <button onClick={() => handleInviteClick(u)} className="h-8 px-4 text-xs font-bold uppercase tracking-wider border border-ren-charcoal/20 text-ren-charcoal hover:bg-ren-charcoal hover:text-white rounded-lg transition-all">
                                                    Summon
                                                </button>
                                            </motion.div>
                                        ))
                                    )}
                                </div>
                            </div>
                        </motion.div>
                    </motion.div>
                )}
            </AnimatePresence>

            <motion.div
                initial={{ opacity: 0, scale: 0.98 }}
                animate={{ opacity: 1, scale: 1 }}
                className="max-w-6xl w-full h-[700px] flex shadow-2xl border border-white/40 bg-white/60 backdrop-blur-2xl rounded-3xl overflow-hidden"
            >
                {/* Main Area */}
                <div className="flex-1 p-10 flex flex-col relative">
                    <div className="absolute top-0 right-0 p-6 opacity-5 pointer-events-none">
                        <h1 className="text-[120px] font-serif leading-none italic">Room</h1>
                    </div>

                    <div className="mb-10 relative z-10">
                        <div className="inline-block px-3 py-1 bg-ren-charcoal text-white text-[10px] font-bold uppercase tracking-[0.2em] rounded-full mb-3 shadow-lg shadow-ren-charcoal/20">
                            Sanctuary ID: {roomInfo.id}
                        </div>
                        <h2 className="text-5xl font-serif italic text-ren-charcoal tracking-tight mb-2">{roomInfo.name}</h2>
                        <div className="h-1 w-20 bg-ren-gold rounded-full"></div>
                    </div>

                    <div className="flex-1 flex flex-col items-center justify-center relative">
                        {isHost ? (
                            <div className="text-center w-full max-w-sm relative z-10">
                                <motion.button
                                    onClick={onStart}
                                    disabled={members.length < (isMode0 ? 1 : 1)} /* Actually classic mode requires >1 but for dev we allow 1 */
                                    whileHover={{ scale: 1.05 }}
                                    whileTap={{ scale: 0.95 }}
                                    className={`w-full py-6 text-2xl font-serif italic transition-all rounded-2xl shadow-xl ${members.length < 1
                                            ? 'opacity-50 bg-gray-300 text-gray-500 cursor-not-allowed'
                                            : 'bg-ren-gold text-white hover:bg-[#C5A028] shadow-ren-gold/30'
                                        }`}
                                >
                                    {members.length < 1 ? 'Awaiting Souls...' : 'Commence Ritual'}
                                </motion.button>
                                <p className="mt-6 text-ren-charcoal/60 text-sm font-medium tracking-wide">
                                    {members.length < 1 ? "At least one soul is required" : "Sanctuary is ready"}
                                </p>
                            </div>
                        ) : (
                            <div className="text-center space-y-6 relative z-10">
                                <div className="w-20 h-20 border-4 border-ren-border border-t-ren-charcoal rounded-full animate-spin mx-auto opacity-80"></div>
                                <p className="text-ren-charcoal/70 font-serif italic text-lg">Waiting for the Architect to begin...</p>
                            </div>
                        )}
                    </div>

                    <motion.button
                        whileHover={{ x: -4 }}
                        onClick={onLeave}
                        className="self-start text-ren-gray hover:text-red-600 font-medium text-sm flex items-center gap-2 transition-colors mt-auto"
                    >
                        <span>←</span> Depart Sanctuary
                    </motion.button>
                </div>

                {/* Sidebar - Players */}
                <div className="w-96 bg-white/40 border-l border-white/40 p-8 flex flex-col">
                    <div className="flex justify-between items-center mb-8">
                        <h3 className="font-serif italic text-2xl text-ren-charcoal">
                            Assemblage
                        </h3>
                        {isHost && !isMode0 && (
                            <motion.button
                                whileHover={{ scale: 1.05 }} whileTap={{ scale: 0.95 }}
                                onClick={handleOpenInvite}
                                className="h-8 px-4 text-xs font-bold bg-ren-charcoal text-white rounded-full shadow-lg shadow-ren-charcoal/10"
                            >
                                + INVITE
                            </motion.button>
                        )}
                    </div>

                    <div className="text-xs font-bold text-ren-gray uppercase tracking-widest mb-4 flex justify-between">
                        <span>Members</span>
                        <span>{members.length}/{maxPlayers}</span>
                    </div>

                    <div className="space-y-4 flex-1 overflow-y-auto pr-2 custom-scrollbar">
                        <AnimatePresence>
                            {members.map((mem, idx) => (
                                <motion.div
                                    key={mem.username || idx}
                                    initial={{ opacity: 0, x: 20 }}
                                    animate={{ opacity: 1, x: 0 }}
                                    exit={{ opacity: 0, x: -20 }}
                                    transition={{ delay: idx * 0.1 }}
                                    className={`relative p-4 flex items-center gap-4 bg-white/80 backdrop-blur-md rounded-xl border transition-all hover:shadow-lg group ${mem.isHost ? 'border-ren-gold/50 shadow-ren-gold/5' : 'border-white/60 shadow-sm'
                                        }`}
                                >
                                    <div className={`w-12 h-12 rounded-full flex items-center justify-center text-white font-serif italic text-lg shadow-md ${mem.isHost ? 'bg-gradient-to-br from-ren-gold to-[#B89020]' : 'bg-ren-charcoal'
                                        }`}>
                                        {mem.username.charAt(0).toUpperCase()}
                                    </div>
                                    <div className="flex-1 min-w-0">
                                        <div className="flex items-center gap-2 mb-1">
                                            <span className="font-bold text-ren-charcoal truncate text-lg group-hover:text-ren-gold transition-colors">{mem.username}</span>
                                        </div>
                                        <div className="text-xs text-ren-gray font-medium flex items-center gap-2">
                                            Score: <span className="text-ren-charcoal font-bold">{mem.score}</span>
                                            {mem.isHost && (
                                                <span className="px-1.5 py-0.5 bg-ren-gold/10 text-ren-gold rounded text-[9px] font-bold uppercase tracking-wider border border-ren-gold/20">Architect</span>
                                            )}
                                        </div>
                                    </div>
                                </motion.div>
                            ))}
                        </AnimatePresence>

                        {!isMode0 && Array.from({ length: Math.max(0, maxPlayers - members.length) }).map((_, i) => (
                            <div key={`empty-${i}`} className="p-4 border-2 border-dashed border-gray-200/50 rounded-xl flex items-center gap-4 opacity-30">
                                <div className="w-12 h-12 rounded-full bg-gray-200"></div>
                                <div className="h-4 bg-gray-200 w-24 rounded"></div>
                            </div>
                        ))}
                    </div>
                </div>
            </motion.div>
        </React.Fragment>
    );
};

export default RoomPage;
