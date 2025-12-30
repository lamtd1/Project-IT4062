import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import { motion, useAnimation, AnimatePresence } from 'framer-motion';

// --- COSMIC BACKGROUND COMPONENT ---
const CosmicBackground = ({ isWarping }) => {
    // Generate static stars for performance
    const [stars] = useState(() => Array.from({ length: 100 }).map((_, i) => ({
        id: i,
        x: Math.random() * 100,
        y: Math.random() * 100,
        size: Math.random() * 2 + 1,
        // When warping, they become lines (scaleX/Y)
    })));

    return (
        <div className="absolute inset-0 overflow-hidden bg-[#05080a]">
            {/* Deep Atmospheric Gradient */}
            <div className="absolute inset-0 bg-gradient-to-b from-[#0f172a] via-[#05080a] to-[#000000] opacity-80" />

            {/* Warp Speed Effect Overlay */}
            <AnimatePresence>
                {isWarping && (
                    <motion.div
                        initial={{ opacity: 0 }}
                        animate={{ opacity: 1 }}
                        transition={{ duration: 1 }}
                        className="absolute inset-0 z-20 bg-black/40 mix-blend-hard-light"
                    >
                        {/* Radial Blur / Speed Lines could go here */}
                    </motion.div>
                )}
            </AnimatePresence>

            {/* Simulated 3D Starfield */}
            <div className={`absolute inset-0 perspective-[1000px] transition-transform duration-[5000ms] ease-in ${isWarping ? 'scale-[5]' : 'scale-100'}`}>
                {stars.map((star) => (
                    <motion.div
                        key={star.id}
                        className="absolute bg-white rounded-full mix-blend-screen"
                        style={{
                            left: `${star.x}%`,
                            top: `${star.y}%`,
                            width: star.size,
                            height: star.size,
                        }}
                        animate={isWarping ? {
                            scaleY: [1, 20],
                            opacity: [1, 0.5],
                            y: [0, 1000] // Fly downwards/outwards simul
                        } : {
                            opacity: [0.2, 0.8, 0.2],
                            scale: [1, 1.2, 1],
                        }}
                        transition={isWarping ? {
                            duration: 0.5,
                            repeat: Infinity,
                            ease: "linear"
                        } : {
                            duration: Math.random() * 3 + 2,
                            repeat: Infinity,
                            ease: "easeInOut",
                        }}
                    />
                ))}
            </div>

            {/* Central Light / Portal Endpoint */}
            <AnimatePresence>
                {isWarping && (
                    <motion.div
                        className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-1 h-1 bg-white rounded-full shadow-[0_0_100px_50px_rgba(255,255,255,1)] z-30"
                        initial={{ scale: 0, opacity: 0 }}
                        animate={{ scale: 300, opacity: 1 }}
                        transition={{ duration: 4, ease: "circIn", delay: 0.2 }}
                    />
                )}
            </AnimatePresence>
        </div>
    );
};

// --- LOGIN PAGE ---
const LoginPage = ({ username, setUsername, password, setPassword, onSubmit, status, userRole, userId }) => { // Add userId/role props
    const [isExiting, setIsExiting] = useState(false);
    const [isChecking, setIsChecking] = useState(false);
    const navigate = useNavigate(); // Requires react-router-dom import

    // 1. Check existing session on mount
    useEffect(() => {
        if (userId) {
            // If already logged in client-side, redirect immediately or asking to continue?
            // For now, let's redirect to save time, or show "Already Logged In"
            navigate(userRole === 0 ? '/admin' : '/home');
        }
    }, [userId, userRole, navigate]);

    // 2. Handle Server Response
    useEffect(() => {
        if (status.type === 'success' && !isExiting) {
            setIsChecking(false);
            setIsExiting(true);
            // Trigger navigation after animation
            setTimeout(() => {
                navigate(userRole === 0 ? '/admin' : '/home');
            }, 4000); // 4s matches animation duration
        } else if (status.type === 'error') {
            setIsChecking(false);
            // Error displayed via status.msg, simple as that
        }
    }, [status, isExiting, navigate, userRole]);

    const handleEnter = () => {
        if (!username || !password) return;
        setIsChecking(true);
        onSubmit(0x01); // Send immediately
    };

    return (
        <div className="relative w-full h-screen overflow-hidden flex items-center justify-center font-serif text-white selection:bg-ren-gold selection:text-black">
            <CosmicBackground isWarping={isExiting} />

            {/* --- MAIN CONTENT LAYER (Centered) --- */}
            <AnimatePresence>
                {!isExiting && (
                    <motion.div
                        initial={{ opacity: 0, scale: 0.95 }}
                        animate={{ opacity: 1, scale: 1 }}
                        exit={{ opacity: 0, scale: 1.1, filter: "blur(20px)" }}
                        transition={{ duration: 0.8 }}
                        className="relative z-10 w-full max-w-md px-6 flex flex-col items-center justify-center space-y-10"
                    >
                        {/* ... BRANDING ... */}
                        <div className="text-center space-y-4">
                            <motion.div
                                animate={{ rotate: 360 }}
                                transition={{ duration: 20, repeat: Infinity, ease: "linear" }}
                                className="w-16 h-16 mx-auto bg-gradient-to-br from-ren-gold to-transparent rounded-full opacity-80 blur-sm"
                            />
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

                        {/* LOGIN CARD */}
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

                            {/* Error */}
                            {status.msg && (
                                <div className={`mt-4 text-xs text-center font-mono uppercase tracking-wide ${status.type === 'error' ? 'text-red-400' : 'text-green-400'}`}>
                                    {status.msg}
                                </div>
                            )}

                            {/* Action */}
                            <div className="mt-8">
                                <button
                                    onClick={handleEnter}
                                    disabled={isChecking}
                                    className="w-full py-4 bg-white text-black font-bold uppercase tracking-widest rounded-xl hover:bg-ren-gold hover:scale-[1.02] transition-all shadow-[0_0_20px_rgba(255,255,255,0.2)] disabled:opacity-50 disabled:cursor-not-allowed flex justify-center items-center gap-2"
                                >
                                    {isChecking && <div className="w-4 h-4 border-2 border-black border-t-transparent rounded-full animate-spin" />}
                                    {isChecking ? "Verifying..." : "Enter"}
                                </button>
                            </div>

                            <div className="mt-8 text-center">
                                <p className="text-[9px] text-white/40 uppercase tracking-widest mb-3">No Credentials?</p>
                                <Link to="/register" className="inline-block px-6 py-2 border border-white/20 rounded-full text-[10px] uppercase tracking-widest text-ren-gold hover:bg-ren-gold hover:text-black hover:border-ren-gold transition-all">
                                    Initiate Registration
                                </Link>
                            </div>
                        </div>

                        <p className="text-[10px] text-gray-500 font-mono">
                            v2.6.0 • SECURE CONNECTION
                        </p>
                    </motion.div>
                )}
            </AnimatePresence>

            {/* CINEMATIC TEXT DURING WARP */}
            <AnimatePresence>
                {isExiting && (
                    <motion.div
                        className="absolute inset-0 z-40 flex items-center justify-center pointer-events-none"
                    >
                        <motion.div
                            initial={{ opacity: 0, scale: 0.8 }}
                            animate={{ opacity: [0, 1, 1, 0], scale: [0.8, 1, 1.2, 1.5], filter: ["blur(10px)", "blur(0px)", "blur(0px)", "blur(10px)"] }}
                            transition={{ duration: 4, times: [0, 0.2, 0.8, 1] }}
                            className="text-center"
                        >
                            <h1 className="text-6xl md:text-9xl font-serif italic text-white drop-shadow-[0_0_30px_rgba(255,255,255,0.5)]">
                                The Millionaire
                            </h1>
                            <p className="text-xl font-sans tracking-[0.5em] text-ren-gold uppercase mt-6 opacity-80">
                                Sanctuary Awaits
                            </p>
                        </motion.div>
                    </motion.div>
                )}
            </AnimatePresence>

        </div>
    );
};

export default LoginPage;
