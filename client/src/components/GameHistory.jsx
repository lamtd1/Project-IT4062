import React, { useState, useEffect } from 'react';
import { socket } from '../lib/socket';
import { OPS } from '../lib/ops';
import ReplayModal from './ReplayModal';

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
                    const [gameId, roomName, winnerId, timestamp, gameMode, score, rank, logData] = entry.split('|');
                    return {
                        gameId: parseInt(gameId),
                        roomName,
                        winnerId: parseInt(winnerId),
                        timestamp,
                        gameMode: parseInt(gameMode),
                        score: parseInt(score),
                        rank: parseInt(rank),
                        logData
                    };
                });

                setGames(parsedGames);
                setLoading(false);
            }
        };

        socket.on('server_to_client', handleServerMessage);
        return () => socket.off('server_to_client', handleServerMessage);
    }, []);

    const getGameModeName = (mode) => {
        switch (mode) {
            case 0: return '🎯 Practice';
            case 1: return '⚔️ Elimination';
            case 2: return '⚡ Score Attack';
            default: return 'Unknown';
        }
    };

    const formatDate = (timestamp) => {
        const date = new Date(timestamp);
        return date.toLocaleString('vi-VN', {
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
            hour: '2-digit',
            minute: '2-digit'
        });
    };

    if (loading) {
        return (
            <div className="p-8 max-w-7xl mx-auto bg-white min-h-screen">
                <div className="text-center py-12 text-gray-600 text-xl">Loading game history...</div>
            </div>
        );
    }

    return (
        <div className="p-8 max-w-7xl mx-auto bg-white min-h-screen">
            <h1 className="text-4xl font-bold mb-8 text-black">📜 Game History</h1>

            {games.length === 0 ? (
                <div className="text-center py-16 px-8 bg-gray-100 border-2 border-gray-300 rounded-xl">
                    <p className="text-gray-600 text-lg">No games played yet. Start playing to build your history!</p>
                </div>
            ) : (
                <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
                    {games.map(game => (
                        <div
                            key={game.gameId}
                            className="bg-white border-2 border-black rounded-xl p-6 hover:shadow-xl hover:-translate-y-1 transition-all duration-200"
                        >
                            <div className="flex justify-between items-center mb-4 pb-3 border-b-2 border-gray-300">
                                <span className="font-semibold text-black">{getGameModeName(game.gameMode)}</span>
                                {game.winnerId === userId && (
                                    <span className="bg-yellow-400 text-black px-2 py-1 rounded-lg text-sm font-bold border border-black">
                                        👑 Winner
                                    </span>
                                )}
                            </div>

                            <div className="space-y-2 mb-4">
                                <p className="font-semibold text-black">🏠 {game.roomName}</p>
                                <p className="text-gray-700 text-sm">📅 {formatDate(game.timestamp)}</p>
                                <p className="text-gray-700">
                                    🎯 Score: <strong className="text-indigo-600 text-lg">{game.score}</strong>
                                </p>
                            </div>

                            <button
                                onClick={() => setReplayGame(game)}
                                className="w-full py-3 bg-black text-white font-semibold rounded-lg border-2 border-black hover:bg-gray-800 transition-all duration-200 hover:scale-105"
                            >
                                ▶️ Replay
                            </button>
                        </div>
                    ))}
                </div>
            )}

            {replayGame && (
                <ReplayModal
                    game={replayGame}
                    userId={userId}
                    onClose={() => setReplayGame(null)}
                />
            )}
        </div>
    );
};

export default GameHistory;
