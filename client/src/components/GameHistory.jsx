import React, { useState, useEffect } from 'react';
import { socket } from '../lib/socket';
import { OPS } from '../lib/ops';
import ReplayModal from './ReplayModal';
import { motion } from 'framer-motion';

const GameHistory = ({ userId }) => {
    const [games, setGames] = useState([]);
    const [loading, setLoading] = useState(true);
    const [replayGame, setReplayGame] = useState(null);

    useEffect(() => {
        const packet = new Uint8Array(1);
        packet[0] = OPS.GET_GAME_HISTORY;
        socket.emit('client_to_server', packet);

        const handleServerMessage = (data) => {
            const view = new Uint8Array(data);
            const opcode = view[0];

            if (opcode === OPS.GAME_HISTORY_RESPONSE) {
                const textDecoder = new TextDecoder();
                const payload = textDecoder.decode(view.slice(1));

                if (!payload) {
                    setGames([]);
                    setLoading(false);
                    return;
                }

                const gameEntries = payload.split(';').filter(e => e);
                const parsedGames = gameEntries.map(entry => {
                    const [gameId, roomName, winnerId, timestamp, gameMode, score, logData] = entry.split('|');
                    return {
                        gameId: parseInt(gameId),
                        roomName,
                        winnerId: parseInt(winnerId),
                        timestamp,
                        gameMode: parseInt(gameMode),
                        score: parseInt(score),
                        logData
                    };
                });

                setGames(parsedGames.reverse()); // Show newest first
                setLoading(false);
            }
        };

        socket.on('server_to_client', handleServerMessage);
        return () => socket.off('server_to_client', handleServerMessage);
    }, []);

    const getModeName = (mode) => {
        switch (mode) {
            case 0: return 'SOLO';
            case 1: return 'COOP';
            case 2: return 'SPEED';
            default: return 'UNKNOWN';
        }
    };

    const formatDate = (timestamp) => {
        const date = new Date(timestamp);
        return date.toLocaleDateString('en-GB', { day: 'numeric', month: 'short' }); // "24 Dec"
    };

    if (loading) {
        return (
            <div className="flex items-center justify-center min-h-[50vh]">
                <div className="w-8 h-8 border-2 border-[#1A1A1A] border-t-transparent rounded-full animate-spin" />
            </div>
        );
    }

    return (
        <div className="max-w-7xl mx-auto p-8 min-h-screen">
            <header className="mb-12 border-b border-[#1A1A1A] pb-4 flex justify-between items-end">
                <div>
                    <h1 className="text-4xl font-serif italic text-[#1A1A1A]">Archives</h1>
                    <p className="text-xs font-mono uppercase tracking-widest text-ren-gray mt-1">
                        Recorded Rituals & Echoes
                    </p>
                </div>
                <div className="text-xs font-mono">
                    TOTAL RECORDS: {games.length}
                </div>
            </header>

            {games.length === 0 ? (
                <div className="flex flex-col items-center justify-center py-20 opacity-50">
                    <span className="text-6xl mb-4 font-serif italic text-gray-300">∅</span>
                    <p className="font-serif text-xl text-gray-400">The archives are empty.</p>
                </div>
            ) : (
                <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
                    {games.map((game, idx) => (
                        <motion.div
                            key={game.gameId}
                            initial={{ opacity: 0, y: 20 }}
                            animate={{ opacity: 1, y: 0 }}
                            transition={{ delay: idx * 0.05 }}
                            className="group bg-white border border-[#1A1A1A] p-6 hover:shadow-[8px_8px_0px_0px_rgba(26,26,26,1)] hover:-translate-y-1 transition-all duration-300 relative overflow-hidden"
                        >
                            {/* Success Gradient if Winner */}
                            {game.winnerId === userId && (
                                <div className="absolute top-0 right-0 w-16 h-16 bg-gradient-to-bl from-ren-gold/30 to-transparent pointer-events-none" />
                            )}

                            <div className="flex justify-between items-start mb-8">
                                <div>
                                    <h3 className="font-serif font-medium italic text-3xl text-[#1A1A1A] truncate max-w-[200px] mb-2 leading-none">
                                        {game.roomName.split(':')[0]}
                                    </h3>
                                    <span className="text-[10px] uppercase tracking-[0.2em] text-ren-gray">
                                        Data Log: {game.gameId}
                                    </span>
                                </div>
                                <div className="px-2 py-1 border border-[#1A1A1A] text-[10px] font-bold uppercase tracking-widest">
                                    {getModeName(game.gameMode)}
                                </div>
                            </div>

                            <div className="space-y-3 mb-8 font-serif text-lg text-gray-800">
                                <div className="flex justify-between border-b border-[#1A1A1A]/10 pb-2">
                                    <span className="italic text-gray-500">Date Recorded</span>
                                    <span className="font-mono text-base">{formatDate(game.timestamp)}</span>
                                </div>
                                <div className="flex justify-between border-b border-[#1A1A1A]/10 pb-2">
                                    <span className="italic text-gray-500">Score Achieved</span>
                                    <span className="font-bold text-[#1A1A1A]">{game.score}</span>
                                </div>
                            </div>

                            <button
                                onClick={() => setReplayGame(game)}
                                className="w-full py-3 border border-[#1A1A1A] text-[#1A1A1A] text-xs font-bold uppercase tracking-widest hover:bg-[#1A1A1A] hover:text-white transition-colors"
                            >
                                Replay Memory
                            </button>
                        </motion.div>
                    ))}
                </div>
            )}

            {replayGame && (
                <ReplayModal
                    game={replayGame}
                    userId={userId}
                    socket={socket} // Pass socket prop freshly
                    onClose={() => setReplayGame(null)}
                />
            )}
        </div>
    );
};

export default GameHistory;
