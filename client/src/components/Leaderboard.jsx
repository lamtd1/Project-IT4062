import React from 'react';
import { motion } from 'framer-motion';

const Leaderboard = ({ leaderboard, onRequest, currentUserId }) => {
    return (
        <div className="flex flex-col h-full">
            <div className="flex-1 overflow-y-auto mb-4 custom-scrollbar">
                <table className="w-full text-left border-collapse">
                    <thead>
                        <tr className="text-[10px] uppercase tracking-widest border-b border-gray-200 text-ren-gray">
                            <th className="py-2 pl-2">Rank</th>
                            <th className="py-2">Player</th>
                            <th className="py-2 text-right pr-2">Score</th>
                        </tr>
                    </thead>
                    <tbody className="text-sm font-mono">
                        {leaderboard.map((user, index) => (
                            <motion.tr
                                key={index}
                                initial={{ opacity: 0, x: -10 }}
                                animate={{ opacity: 1, x: 0 }}
                                whileHover={{ scale: 1.02, backgroundColor: "#fff", zIndex: 10, boxShadow: "0 4px 12px rgba(0,0,0,0.05)" }}
                                transition={{ delay: index * 0.05 }}
                                className={`border-b border-gray-50 transition-colors cursor-default ${user.name === currentUserId ? 'bg-ren-gold/10' : ''
                                    }`}
                            >
                                <td className="py-3 pl-2 font-bold text-ren-gray/50">#{index + 1}</td>
                                <td className="py-3 font-medium text-ren-charcoal">
                                    {user.name}
                                    {index === 0 && <span className="ml-2 text-ren-gold">👑</span>}
                                </td>
                                <td className="py-3 text-right pr-2 font-bold">{user.score}</td>
                            </motion.tr>
                        ))}
                        {leaderboard.length === 0 && (
                            <tr>
                                <td colSpan="3" className="py-8 text-center text-xs text-ren-gray italic">
                                    No legends yet recorded...
                                </td>
                            </tr>
                        )}
                    </tbody>
                </table>
            </div>

            <motion.button
                whileHover={{ scale: 1.02, backgroundColor: "#1A1A1A", color: "#ffffff" }}
                whileTap={{ scale: 0.98 }}
                onClick={onRequest}
                className="w-full py-3 border border-[#1A1A1A] text-[#1A1A1A] font-bold text-xs uppercase tracking-widest transition-all"
            >
                Refresh Rankings
            </motion.button>
        </div>
    );
};

export default Leaderboard;
