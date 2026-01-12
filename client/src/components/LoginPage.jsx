import React, { useState, useEffect } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { motion, AnimatePresence } from 'framer-motion';

// --- SIMPLE LOGIN PAGE (No Animation) ---
const LoginPage = ({ username, setUsername, password, setPassword, onSubmit, status, userRole, userId }) => {
    const [isChecking, setIsChecking] = useState(false);
    const navigate = useNavigate();

    // Handle Server Response - Navigation is handled by useGameLogic.js
    useEffect(() => {
        if (status.type === 'success') {
            setIsChecking(false);
            // Navigation handled by useGameLogic.js using direct role value
        } else if (status.type === 'error') {
            setIsChecking(false);
        }
    }, [status]);

    const handleEnter = () => {
        if (!username || !password) return;
        setIsChecking(true);
        onSubmit(0x01); // MSG_LOGIN
    };

    return (
        <div className="relative w-full h-screen overflow-hidden flex items-center justify-center font-serif text-white bg-gradient-to-b from-[#0f172a] via-[#05080a] to-[#000000]">
            {/* Simple starfield background */}
            <div className="absolute inset-0">
                {Array.from({ length: 50 }).map((_, i) => (
                    <motion.div
                        key={i}
                        className="absolute bg-white rounded-full"
                        style={{
                            left: `${Math.random() * 100}%`,
                            top: `${Math.random() * 100}%`,
                            width: Math.random() * 2 + 1,
                            height: Math.random() * 2 + 1,
                        }}
                        animate={{
                            opacity: [0.2, 0.8, 0.2],
                        }}
                        transition={{
                            duration: Math.random() * 3 + 2,
                            repeat: Infinity,
                            ease: "easeInOut",
                        }}
                    />
                ))}
            </div>

            {/* Main Content */}
            <motion.div
                initial={{ opacity: 0, scale: 0.95 }}
                animate={{ opacity: 1, scale: 1 }}
                transition={{ duration: 0.5 }}
                className="relative z-10 w-full max-w-md px-6 flex flex-col items-center justify-center space-y-10"
            >
                {/* Branding */}
                <div className="text-center space-y-4">
                    <div className="w-16 h-16 mx-auto bg-gradient-to-br from-ren-gold to-transparent rounded-full opacity-80 blur-sm" />
                    <div className="-mt-12 relative z-10 flex flex-col items-center">
                        <h2 className="text-xs md:text-sm font-sans tracking-[0.4em] uppercase text-white/50 mb-2">
                            Who Wants To Be A
                        </h2>
                        <h1 className="text-5xl md:text-7xl font-serif font-bold italic tracking-tighter text-transparent bg-clip-text bg-gradient-to-b from-ren-gold to-white drop-shadow-2xl">
                            Millionaire
                        </h1>
                        <p className="text-[10px] font-sans tracking-[0.6em] uppercase text-ren-gold/60 mt-4 border-t border-ren-gold/20 pt-2 w-full text-center">
                            Renaissance Edition
                        </p>
                    </div>
                </div>

                {/* Login Card */}
                <div className="w-full relative group backdrop-blur-xl bg-white/5 border border-white/10 p-8 rounded-3xl shadow-2xl">
                    {/* Inputs */}
                    <div className="space-y-6">
                        <div className="space-y-1">
                            <label className="text-[10px] uppercase tracking-wider text-gray-400 font-sans ml-2">Appellation</label>
                            <input
                                type="text"
                                value={username}
                                onChange={(e) => setUsername(e.target.value)}
                                onKeyDown={(e) => e.key === 'Enter' && handleEnter()}
                                className="w-full bg-black/40 border border-white/5 rounded-xl px-4 py-3 text-lg text-center focus:outline-none focus:border-ren-gold/50 focus:bg-black/60 transition-all font-sans placeholder:text-gray-600"
                                placeholder="Identity"
                            />
                        </div>
                        <div className="space-y-1">
                            <label className="text-[10px] uppercase tracking-wider text-gray-400 font-sans ml-2">Cipher</label>
                            <input
                                type="password"
                                value={password}
                                onChange={(e) => setPassword(e.target.value)}
                                onKeyDown={(e) => e.key === 'Enter' && handleEnter()}
                                className="w-full bg-black/40 border border-white/5 rounded-xl px-4 py-3 text-lg text-center focus:outline-none focus:border-ren-gold/50 focus:bg-black/60 transition-all font-sans placeholder:text-gray-600"
                                placeholder="••••••"
                            />
                        </div>
                    </div>

                    {/* Status Message */}
                    {status.msg && (
                        <div className={`mt-4 text-xs text-center font-mono uppercase tracking-wide ${status.type === 'error' ? 'text-red-400' : 'text-green-400'}`}>
                            {status.msg}
                        </div>
                    )}

                    {/* Login Button */}
                    <div className="mt-8">
                        <button
                            onClick={handleEnter}
                            disabled={isChecking}
                            className="w-full py-4 bg-white text-black font-bold uppercase tracking-widest rounded-xl hover:bg-ren-gold hover:scale-[1.02] transition-all shadow-[0_0_20px_rgba(255,255,255,0.2)] disabled:opacity-50 disabled:cursor-not-allowed flex justify-center items-center gap-2"
                        >
                            {isChecking && <div className="w-4 h-4 border-2 border-black border-t-transparent rounded-full animate-spin" />}
                            {isChecking ? "Authenticating..." : "Enter"}
                        </button>
                    </div>

                    {/* Register Link */}
                    <div className="mt-8 text-center">
                        <p className="text-[9px] text-white/40 uppercase tracking-widest mb-3">No credentials?</p>
                        <Link to="/register" className="inline-block px-6 py-2 border border-white/20 rounded-full text-[10px] uppercase tracking-widest text-ren-gold hover:bg-ren-gold hover:text-black hover:border-ren-gold transition-all">
                            initial registration
                        </Link>
                    </div>
                </div>

                <p className="text-[10px] text-gray-500 font-mono">
                    v2.6.0 • SECURE CONNECTION
                </p>
            </motion.div>
        </div>
    );
};

export default LoginPage;
