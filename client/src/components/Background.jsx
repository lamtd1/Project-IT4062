import React from 'react';
import { motion } from 'framer-motion';

const Background = () => {
    return (
        <div className="fixed inset-0 z-[-1] overflow-hidden bg-[#F2F0E9] pointer-events-none">
            {/* 1. Base Gradient (Warm Renaissance Light) */}
            <div className="absolute inset-0 bg-gradient-to-br from-[#F2F0E9] via-[#E6D5B8] to-[#D4C5A8] opacity-80" />

            {/* 2. Classic Art Abstract Shapes (Moving Clouds/Fabric) */}
            <motion.div
                className="absolute top-[-20%] left-[-10%] w-[70vw] h-[70vw] rounded-full blur-[100px] bg-[#C8B085]/30 mix-blend-multiply"
                animate={{ x: [0, 30, 0], y: [0, 40, 0], scale: [1, 1.05, 1] }}
                transition={{ duration: 25, repeat: Infinity, ease: "easeInOut" }}
            />
            <motion.div
                className="absolute bottom-[-20%] right-[-10%] w-[60vw] h-[60vw] rounded-full blur-[120px] bg-[#A89F91]/40 mix-blend-multiply"
                animate={{ x: [0, -40, 0], y: [0, -30, 0], scale: [1, 1.1, 1] }}
                transition={{ duration: 30, repeat: Infinity, ease: "easeInOut", delay: 2 }}
            />

            {/* 3. Golden Dust / Particles Effect */}
            <div className="absolute inset-0 z-[1] overflow-hidden">
                {Array.from({ length: 20 }).map((_, i) => (
                    <motion.div
                        key={i}
                        className="absolute rounded-full bg-ren-gold/40 blur-[1px]"
                        style={{
                            width: Math.random() * 4 + 1 + 'px',
                            height: Math.random() * 4 + 1 + 'px',
                            left: Math.random() * 100 + '%',
                            top: Math.random() * 100 + '%'
                        }}
                        animate={{
                            y: [0, -100, 0],
                            opacity: [0, 0.8, 0],
                            scale: [0.5, 1.2, 0.5]
                        }}
                        transition={{
                            duration: Math.random() * 10 + 10,
                            repeat: Infinity,
                            ease: "linear",
                            delay: Math.random() * 5
                        }}
                    />
                ))}
            </div>

            {/* 4. Rotating Sacred Geometry / Astrolabe Lines */}
            <div className="absolute inset-0 z-[0] flex items-center justify-center opacity-[0.03] pointer-events-none">
                <motion.div
                    className="w-[80vw] h-[80vw] border border-[#1A1A1A] rounded-full"
                    animate={{ rotate: 360 }}
                    transition={{ duration: 120, repeat: Infinity, ease: "linear" }}
                />
                <motion.div
                    className="absolute w-[60vw] h-[60vw] border border-[#1A1A1A] rounded-full"
                    animate={{ rotate: -360 }}
                    transition={{ duration: 90, repeat: Infinity, ease: "linear" }}
                />
                <motion.div
                    className="absolute w-[40vw] h-[40vw] border border-dashed border-[#1A1A1A] rounded-full"
                    animate={{ rotate: 180 }}
                    transition={{ duration: 60, repeat: Infinity, ease: "linear" }}
                />
            </div>

            {/* 5. Digital Grid Overlay (Subtle) */}
            <div
                className="absolute inset-0 opacity-[0.03] z-[2]"
                style={{
                    backgroundImage: `linear-gradient(#1A1A1A 1px, transparent 1px), linear-gradient(90deg, #1A1A1A 1px, transparent 1px)`,
                    backgroundSize: '40px 40px'
                }}
            />

            {/* 4. Film Grain / Noise Texture (Old Painting feel) */}
            <div className="absolute inset-0 opacity-[0.06] mix-blend-overlay"
                style={{ backgroundImage: `url("data:image/svg+xml,%3Csvg viewBox='0 0 200 200' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='noiseFilter'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.8' numOctaves='3' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23noiseFilter)'/%3E%3C/svg%3E")` }}
            />

            {/* 5. Random Digital Glitches (The 'AI' part) */}
            <motion.div
                className="absolute top-[30%] left-[20%] w-[1px] h-[100px] bg-red-400/40"
                animate={{ opacity: [0, 1, 0], height: [0, 150, 0] }}
                transition={{ duration: 3, repeat: Infinity, repeatDelay: 5 }}
            />
            <motion.div
                className="absolute bottom-[20%] right-[30%] w-[100px] h-[1px] bg-indigo-400/30"
                animate={{ opacity: [0, 1, 0], width: [0, 200, 0] }}
                transition={{ duration: 4, repeat: Infinity, repeatDelay: 7 }}
            />
        </div>
    );
};

export default Background;
