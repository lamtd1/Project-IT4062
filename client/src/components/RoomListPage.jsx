import React from 'react';
import { motion } from 'framer-motion';

const RoomListPage = ({ rooms, onJoin }) => {
    if (!rooms || rooms.length === 0) {
        return (
            <div className="h-full flex flex-col items-center justify-center text-ren-gray space-y-4">
                <div className="w-16 h-16 border border-ren-gray/30 rounded-full flex items-center justify-center italic font-serif text-2xl opacity-50">
                    ∅
                </div>
                <p className="font-serif italic text-xl">The void is silent...</p>
                <p className="text-xs font-mono uppercase tracking-widest opacity-60">Create a sanctuary to begin</p>
            </div>
        );
    }

    return (
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {rooms.map((room, idx) => {
                const isFull = room.count >= 4;
                const isPlaying = room.status === 'PLAYING';

                return (
                    <motion.div
                        key={room.id}
                        initial={{ opacity: 0, y: 20 }}
                        whileInView={{ opacity: 1, y: 0 }}
                        viewport={{ once: true, margin: "-50px" }}
                        transition={{ delay: idx * 0.05 }}
                        whileHover={!isPlaying ? { y: -5, boxShadow: "4px 4px 0px 0px rgba(212,175,55,1)" } : {}}
                        className={`group relative p-6 border transition-all duration-300 ${isPlaying
                            ? 'bg-gray-100/90 border-gray-300 opacity-70'
                            : 'bg-white/80 backdrop-blur-sm border-[#1A1A1A]'
                            }`}
                    >
                        <div className="flex justify-between items-start mb-4">
                            <div>
                                <span className={`inline-block px-2 py-0.5 text-[9px] font-bold uppercase tracking-wider mb-2 border ${isPlaying ? 'border-gray-400 text-gray-500' : 'border-ren-charcoal text-ren-charcoal'
                                    }`}>
                                    ID: {room.id.toString().padStart(3, '0')}
                                </span>
                                <h3 className="font-serif font-bold text-xl text-ren-charcoal group-hover:text-ren-gold transition-colors truncate max-w-[180px]">
                                    {room.name.split(':')[0]}
                                </h3>
                            </div>
                            <div className="flex flex-col items-end">
                                <span className="text-2xl">{getModeIcon(room.name)}</span>
                            </div>
                        </div>

                        <div className="flex justify-between items-end mt-4 pt-4 border-t border-gray-100">
                            <div className="flex items-center gap-2 text-xs font-mono text-gray-500">
                                <span className={`w-2 h-2 rounded-full ${isPlaying ? 'bg-orange-400' : 'bg-green-500'}`} />
                                <span className={isPlaying ? 'animate-pulse' : ''}>{isPlaying ? 'IN PROGRESS' : 'OPEN'}</span>
                                <span>•</span>
                                <span>{room.count}/4</span>
                            </div>

                            {!isPlaying && !isFull && (
                                <motion.button
                                    whileHover={{ scale: 1.05 }}
                                    whileTap={{ scale: 0.95 }}
                                    onClick={() => onJoin(room.id)}
                                    className="bg-[#1A1A1A] text-white px-4 py-2 text-xs font-bold uppercase tracking-wider hover:bg-ren-gold transition-colors"
                                >
                                    Join
                                </motion.button>
                            )}
                        </div>
                    </motion.div>
                );
            })}
        </div>
    );
};

// Helper for icons based on room name suffix format "Name:Mode"
const getModeIcon = (roomName) => {
    if (roomName.includes(':0')) return '👤'; // Practice
    if (roomName.includes(':1')) return '🤝'; // Coop
    if (roomName.includes(':2')) return '⚡'; // Score
    return '🎲';
};

export default RoomListPage;
