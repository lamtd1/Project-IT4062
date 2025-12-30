import React, { useState } from 'react';
import { Link } from 'react-router-dom';
import { motion, AnimatePresence } from 'framer-motion';

const RegisterPage = ({ username, setUsername, password, setPassword, onSubmit, status }) => {
    const [isExiting, setIsExiting] = useState(false);
    const [isChecking, setIsChecking] = useState(false);
    const navigate = useNavigate();

    useEffect(() => {
        if (status.type === 'success' && !isExiting) {
            setIsChecking(false);
            setIsExiting(true);
            setTimeout(() => navigate('/'), 1000); // Wait for exit animation
        } else if (status.type === 'error') {
            setIsChecking(false);
        }
    }, [status, isExiting, navigate]);

    const handleRegister = () => {
        if (!username || !password) return;
        setIsChecking(true);
        onSubmit(0x02);
    };

    return (
        <div className="w-full h-full flex items-center justify-center relative overflow-hidden">
            {/* PORTAL EFFECT OVERLAY (Reverse) */}
            <AnimatePresence>
                {isExiting && (
                    <motion.div
                        className="fixed inset-0 z-50 flex items-center justify-center bg-white pointer-events-none"
                        initial={{ opacity: 0 }}
                        animate={{ opacity: 1 }}
                        exit={{ opacity: 0 }}
                    >
                        <motion.div
                            className="w-[200vw] h-[200vw] rounded-full border-[50px] border-[#1A1A1A]/20"
                            initial={{ scale: 2, opacity: 0 }}
                            animate={{ scale: 0, opacity: 1 }}
                            transition={{ duration: 1, ease: "anticipate" }}
                        />
                    </motion.div>
                )}
            </AnimatePresence>

            <motion.div
                initial={{ opacity: 0, scale: 0.95 }}
                animate={{ opacity: 1, scale: 1 }}
                transition={{ duration: 0.8, ease: "easeOut" }}
                className="relative z-10 w-full max-w-md bg-white border border-[#1A1A1A] p-12 shadow-[10px_10px_0px_0px_rgba(26,26,26,1)]"
            >
                {/* Decorative Elements */}
                <div className="absolute top-0 left-0 w-full h-1 bg-[#1A1A1A]" />
                <div className="absolute bottom-0 left-0 w-full h-1 bg-ren-gold" />

                <div className="text-center mb-10">
                    <h2 className="text-4xl font-serif italic text-[#1A1A1A] mb-2">Initiation</h2>
                    <p className="text-[10px] uppercase tracking-[0.3em] font-medium text-gray-500">
                        Create Your Legacy
                    </p>
                </div>

                {status.msg && (
                    <motion.div
                        initial={{ opacity: 0, y: -10 }}
                        animate={{ opacity: 1, y: 0 }}
                        className={`mb-6 p-3 text-xs font-mono border-l-2 ${status.type === 'success'
                            ? 'border-green-500 bg-green-50 text-green-700'
                            : 'border-red-500 bg-red-50 text-red-600'
                            }`}
                    >
                        [{status.type === 'success' ? 'SUCCESS' : 'ERROR'}]: {status.msg}
                    </motion.div>
                )}

                <div className="space-y-8">
                    <div className="group relative">
                        <label className="text-[9px] uppercase tracking-widest text-ren-gray mb-1 block">
                            Chosen Name
                        </label>
                        <input
                            type="text"
                            value={username}
                            onChange={(e) => setUsername(e.target.value)}
                            className="block w-full border-b border-gray-300 py-2 text-lg font-serif bg-transparent focus:border-ren-gold focus:outline-none transition-colors"
                        />
                    </div>

                    <div className="group relative">
                        <label className="text-[9px] uppercase tracking-widest text-ren-gray mb-1 block">
                            Secret Key
                        </label>
                        <input
                            type="password"
                            value={password}
                            onChange={(e) => setPassword(e.target.value)}
                            className="block w-full border-b border-gray-300 py-2 text-lg font-serif bg-transparent focus:border-ren-gold focus:outline-none transition-colors"
                        />
                    </div>
                </div>

                <div className="mt-12 space-y-4">
                    <button
                        onClick={handleRegister}
                        disabled={isChecking}
                        className="w-full bg-[#1A1A1A] text-white py-4 font-sans text-xs font-bold uppercase tracking-widest hover:bg-ren-gold hover:text-white transition-colors shadow-lg disabled:opacity-50 flex justify-center items-center gap-2"
                    >
                        {isChecking && <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin" />}
                        {isChecking ? "Forging..." : "Forged In Code"}
                    </button>

                    <div className="flex justify-center">
                        <Link
                            to="/"
                            className="text-[10px] uppercase tracking-wider text-gray-400 hover:text-[#1A1A1A] hover:underline underline-offset-4 transition-all"
                        >
                            Return to Login Portal
                        </Link>
                    </div>
                </div>
            </motion.div>
        </div>
    );
};

export default RegisterPage;
