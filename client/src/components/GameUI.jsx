import React, { useState, useEffect } from 'react';
import { motion, AnimatePresence } from 'framer-motion';

const GameUI = ({
    question,
    timer,
    score,
    onAnswer,
    useLifeline,
    onLeave,
    disableAnswer,
    isAudienceUsed,
    is5050Used,
    isPhoneUsed,
    isExpertUsed,
    room // Pass room info if needed for mode specifics (e.g. Coop)
}) => {
    // Local state for visual feedback
    const [selectedAns, setSelectedAns] = useState(null);
    const [revealResult, setRevealResult] = useState(null); // 'correct' or 'wrong'

    // Reset local state when question changes
    useEffect(() => {
        setSelectedAns(null);
        setRevealResult(null);
    }, [question]);

    const handleAnswerClick = (ansKey) => {
        if (disableAnswer || selectedAns) return;
        setSelectedAns(ansKey);
        onAnswer(ansKey);
    };

    // Helper to get answer text safely
    const getAnswerText = (key) => {
        if (!question || !question.answers) return "";
        switch (key) {
            case 'A': return question.answers[0];
            case 'B': return question.answers[1];
            case 'C': return question.answers[2];
            case 'D': return question.answers[3];
            default: return "";
        }
    };

    if (!question) return (
        <div className="flex items-center justify-center h-full">
            <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-ren-gold"></div>
        </div>
    );

    return (
        <div className="relative w-full h-full flex flex-col overflow-hidden text-[#1A1A1A]">

            {/* --- TOP BAR: HUD --- */}
            <div className="flex justify-between items-center p-6 border-b border-[#1A1A1A]/10 bg-white/80 backdrop-blur-sm z-20">
                <div className="flex items-center gap-4">
                    <div className="w-12 h-12 rounded-full bg-[#1A1A1A] text-white flex items-center justify-center font-serif font-bold text-xl border-2 border-double border-ren-gold">
                        {timer}
                    </div>
                    <div className="flex flex-col">
                        <span className="text-[10px] uppercase tracking-widest text-ren-gray">Time Remaining</span>
                        <div className="h-1 w-32 bg-gray-200 mt-1 overflow-hidden">
                            <motion.div
                                className="h-full bg-ren-gold"
                                initial={{ width: "100%" }}
                                animate={{ width: `${(timer / 30) * 100}%` }}
                                transition={{ duration: 1, ease: "linear" }}
                            />
                        </div>
                    </div>
                </div>

                <div className="text-center absolute left-1/2 -translate-x-1/2">
                    <h2 className="font-serif italic text-2xl">Quest {question.id}</h2>
                    <span className="text-[9px] font-mono uppercase tracking-[0.2em] text-ren-gray">Renaissance Intellect</span>
                </div>

                <div className="flex items-center gap-4">
                    <div className="text-right">
                        <span className="block text-[10px] uppercase tracking-widest text-ren-gray">Total Score</span>
                        <span className="font-serif font-bold text-xl">{score}</span>
                    </div>
                </div>
            </div>

            {/* --- MAIN STAGE: QUESTION --- */}
            <div className="flex-1 flex items-center justify-center p-8 relative z-10">
                <div className="max-w-4xl text-center relative">
                    {/* Decorative Quotes */}
                    <span className="absolute -top-12 left-0 text-9xl font-serif text-ren-gold/10 opacity-50">“</span>

                    <AnimatePresence mode="wait">
                        <motion.h1
                            key={question.id}
                            initial={{ opacity: 0, y: 20, filter: "blur(10px)" }}
                            animate={{ opacity: 1, y: 0, filter: "blur(0px)" }}
                            exit={{ opacity: 0, y: -20, filter: "blur(10px)" }}
                            transition={{ duration: 0.8, ease: "circOut" }}
                            className="text-4xl md:text-5xl font-serif font-medium leading-tight text-[#1A1A1A]"
                        >
                            {question.content}
                        </motion.h1>
                    </AnimatePresence>

                    <span className="absolute -bottom-16 right-0 text-9xl font-serif text-ren-gold/10 opacity-50 rotate-180">“</span>
                </div>
            </div>

            {/* --- BOTTOM: ANSWERS & LIFELINES --- */}
            <div className="p-6 pb-20 z-20 relative"> {/* Added pb-20 for bottom footer space */}
                {/* Lifelines Bar */}
                <div className="flex justify-center gap-4 mb-8">
                    {[
                        { id: 1, label: "50:50", icon: "🌗", used: is5050Used },
                        { id: 2, label: "Audience", icon: "👥", used: isAudienceUsed },
                        { id: 3, label: "Call", icon: "📞", used: isPhoneUsed },
                        { id: 4, label: "Expert", icon: "🎓", used: isExpertUsed },
                    ].map((lf) => (
                        <button
                            key={lf.id}
                            onClick={() => useLifeline(lf.id)}
                            disabled={lf.used}
                            className={`
                                group relative w-12 h-12 flex items-center justify-center rounded-full border border-[#1A1A1A] transition-all
                                ${lf.used ? 'opacity-30 cursor-not-allowed bg-gray-200' : 'hover:scale-110 hover:bg-[#1A1A1A] hover:text-white bg-white'}
                            `}
                            title={lf.label}
                        >
                            <span className="text-lg">{lf.icon}</span>
                            {!lf.used && (
                                <span className="absolute -top-8 opacity-0 group-hover:opacity-100 transition-opacity text-[10px] font-bold uppercase bg-black text-white px-2 py-1 rounded">
                                    {lf.label}
                                </span>
                            )}
                        </button>
                    ))}
                </div>

                {/* Answers Grid */}
                <div className="grid grid-cols-1 md:grid-cols-2 gap-4 max-w-5xl mx-auto">
                    {['A', 'B', 'C', 'D'].map((key) => {
                        const isSelected = selectedAns === key;
                        const isDisabled = disableAnswer && !isSelected;

                        return (
                            <motion.button
                                key={key}
                                onClick={() => handleAnswerClick(key)}
                                disabled={disableAnswer}
                                whileHover={!disableAnswer ? { scale: 1.02 } : {}}
                                whileTap={!disableAnswer ? { scale: 0.98 } : {}}
                                className={`
                                    relative overflow-hidden p-6 border text-left transition-all duration-300
                                    ${isSelected
                                        ? 'bg-[#1A1A1A] text-white border-[#1A1A1A] shadow-[8px_8px_0px_0px_rgba(212,175,55,0.8)]'
                                        : 'bg-white/80 border-[#1A1A1A] hover:bg-white hover:shadow-[4px_4px_0px_0px_rgba(26,26,26,1)]'
                                    }
                                    ${isDisabled ? 'opacity-50 blur-[1px]' : 'opacity-100'}
                                `}
                            >
                                <div className="flex items-center gap-4 relative z-10">
                                    <span className={`
                                        flex-shrink-0 w-8 h-8 flex items-center justify-center font-serif italic text-lg border
                                        ${isSelected ? 'border-white text-white' : 'border-[#1A1A1A] text-[#1A1A1A]'}
                                    `}>
                                        {key}
                                    </span>
                                    <span className={`font-medium text-lg ${isSelected ? 'font-serif italic' : 'font-sans'}`}>
                                        {getAnswerText(key)}
                                    </span>
                                </div>

                                {/* Background Glitch Effect on Hover (only if active) */}
                                {!disableAnswer && !isSelected && (
                                    <div className="absolute inset-0 bg-ren-gold/10 opacity-0 hover:opacity-100 transition-opacity pointer-events-none" />
                                )}
                            </motion.button>
                        );
                    })}
                </div>
            </div>

            {/* Bottom Controls / Footer Area */}
            <div className="absolute bottom-4 right-4 z-50">
                <button
                    onClick={onLeave}
                    className="flex items-center gap-2 px-4 py-2 bg-white/80 backdrop-blur border border-red-200 text-red-600 hover:bg-red-50 text-[10px] font-bold uppercase tracking-widest rounded-full transition-all shadow-sm hover:shadow-md"
                >
                    <span className="w-2 h-2 rounded-full bg-red-500 animate-pulse"></span>
                    Surrender
                </button>
            </div>
        </div>
    );
};

export default GameUI;
