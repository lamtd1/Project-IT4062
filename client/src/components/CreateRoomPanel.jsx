import React, { useState } from 'react';
import { motion } from 'framer-motion';

const CreateRoomPanel = ({ onCreate }) => {
    const [name, setName] = useState("");
    const [mode, setMode] = useState("1"); // Default Coop (1)

    return (
        <motion.div
            initial={{ opacity: 0, x: -20 }}
            animate={{ opacity: 1, x: 0 }}
            transition={{ duration: 0.5 }}
            className="w-full bg-white/90 backdrop-blur-sm border border-[#1A1A1A] p-6 shadow-[4px_4px_0px_0px_rgba(26,26,26,1)]"
        >
            <h3 className="text-3xl font-serif italic text-ren-charcoal mb-1">Create</h3>
            <div className="w-12 h-1 bg-ren-gold mb-6" />

            <div className="space-y-6">
                <div className="space-y-2">
                    <label className="text-[10px] font-bold text-ren-gray uppercase tracking-widest">
                        Room Name
                    </label>
                    <motion.input
                        whileFocus={{ scale: 1.02, borderColor: "#1A1A1A" }}
                        type="text"
                        placeholder="Ex: The Colosseum"
                        value={name}
                        onChange={(e) => setName(e.target.value)}
                        className="w-full p-3 bg-gray-50 border border-gray-200 focus:border-[#1A1A1A] outline-none text-sm font-mono transition-colors placeholder:font-serif placeholder:italic"
                    />
                </div>

                <div className="space-y-3">
                    <label className="text-[10px] font-bold text-ren-gray uppercase tracking-widest">
                        Ritual Mode
                    </label>
                    <div className="grid grid-cols-3 gap-2">
                        {[
                            { id: "0", label: "SOLO" },
                            { id: "1", label: "COOP" },
                            { id: "2", label: "SPEED" },
                        ].map((m) => (
                            <motion.button
                                key={m.id}
                                onClick={() => setMode(m.id)}
                                whileHover={{ scale: 1.05, y: -2 }}
                                whileTap={{ scale: 0.95 }}
                                className={`flex flex-col items-center justify-center p-4 border transition-all ${mode === m.id
                                    ? "bg-[#1A1A1A] text-white border-[#1A1A1A]"
                                    : "bg-white text-ren-gray border-gray-200 hover:border-gray-400"
                                    }`}
                            >
                                <span className="text-sm font-bold tracking-widest">{m.label}</span>
                            </motion.button>
                        ))}
                    </div>
                </div>

                <motion.button
                    onClick={() => name && onCreate(`${name}:${mode}`)}
                    disabled={!name}
                    whileHover={{ scale: 1.02, boxShadow: "0px 10px 20px rgba(212, 175, 55, 0.2)" }}
                    whileTap={{ scale: 0.98 }}
                    className="w-full py-4 bg-ren-gold text-white font-serif italic text-lg hover:bg-[#C5A028] disabled:opacity-50 disabled:cursor-not-allowed transition-colors shadow-lg shadow-ren-gold/20"
                >
                    Manifest Sanctuary
                </motion.button>
            </div>
        </motion.div>
    );
}

export default CreateRoomPanel;
