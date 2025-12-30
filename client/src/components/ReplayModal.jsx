import React, { useState, useEffect } from 'react';
import { OPS } from '../lib/ops';
import { motion } from 'framer-motion';

const ReplayModal = ({ game, userId, socket, onClose }) => {
    const [currentStep, setCurrentStep] = useState(0);
    const [isPlaying, setIsPlaying] = useState(false);
    const [steps, setSteps] = useState([]);
    const [questions, setQuestions] = useState({});
    const [questionsLoaded, setQuestionsLoaded] = useState(false);

    // [Same logic fetch as before, just restyled UI]
    useEffect(() => {
        if (!game.logData) {
            setSteps([]);
            setQuestionsLoaded(true);
            return;
        }

        const entries = game.logData.split(',').filter(e => e);
        const parsedSteps = entries.map(entry => {
            const [playerId, questionId, answer] = entry.split(':');
            return { userId: parseInt(playerId), questionId: parseInt(questionId), answer };
        });

        setSteps(parsedSteps);

        const questionIds = [...new Set(parsedSteps.map(s => s.questionId))];
        if (questionIds.length === 0) {
            setQuestionsLoaded(true);
            return;
        }

        const idsString = questionIds.join(',');
        const encoder = new TextEncoder();
        const idsBytes = encoder.encode(idsString);
        const packet = new Uint8Array(1 + idsBytes.length);
        packet[0] = OPS.GET_QUESTIONS_BY_IDS;
        packet.set(idsBytes, 1);
        socket.emit('client_to_server', packet);

        const handleServerMessage = (data) => {
            const view = new Uint8Array(data);
            if (view[0] === OPS.QUESTIONS_RESPONSE) {
                const textDecoder = new TextDecoder();
                const payload = textDecoder.decode(view.slice(1));
                const questionsMap = {};
                if (payload) {
                    payload.split(';').filter(e => e).forEach(entry => {
                        const [id, content, ansA, ansB, ansC, ansD, correct] = entry.split('|');
                        questionsMap[parseInt(id)] = {
                            id: parseInt(id),
                            content,
                            answers: [ansA, ansB, ansC, ansD],
                            correctAnswer: correct
                        };
                    });
                }
                setQuestions(questionsMap);
                setQuestionsLoaded(true);
            }
        };

        socket.on('server_to_client', handleServerMessage);
        return () => socket.off('server_to_client', handleServerMessage);
    }, [game.logData, socket]);

    useEffect(() => {
        if (!isPlaying || currentStep >= steps.length) {
            setIsPlaying(false);
            return;
        }
        const timer = setTimeout(() => setCurrentStep(prev => prev + 1), 2000);
        return () => clearTimeout(timer);
    }, [isPlaying, currentStep, steps.length]);

    const handlePlay = () => {
        if (currentStep >= steps.length) setCurrentStep(0);
        setIsPlaying(true);
    };

    const currentStepData = steps[currentStep];
    const isMyTurn = currentStepData?.userId === userId;
    const currentQuestion = currentStepData ? questions[currentStepData.questionId] : null;

    if (!questionsLoaded) {
        return (
            <div className="fixed inset-0 bg-white/90 backdrop-blur-sm flex items-center justify-center z-[100]">
                <div className="text-ren-charcoal font-serif italic text-xl animate-pulse">Retrieving Memories...</div>
            </div>
        );
    }

    return (
        <motion.div
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            className="fixed inset-0 z-[100] flex items-center justify-center bg-[#F2F0E9]/90 backdrop-blur-md p-4"
            onClick={onClose}
        >
            <div
                className="w-full max-w-5xl h-[80vh] bg-white border border-[#1A1A1A] shadow-[20px_20px_0px_0px_rgba(26,26,26,1)] flex flex-col overflow-hidden"
                onClick={(e) => e.stopPropagation()}
            >
                {/* Header */}
                <header className="flex justify-between items-center p-6 border-b border-[#1A1A1A]">
                    <div>
                        <h2 className="text-2xl font-serif italic text-[#1A1A1A]">{game.roomName}</h2>
                        <div className="flex gap-4 mt-2 text-[10px] uppercase tracking-widest text-ren-gray">
                            <span>Step {currentStep + 1}/{steps.length}</span>
                            <span>•</span>
                            <span>Memory ID: {game.gameId}</span>
                        </div>
                    </div>
                    <button
                        onClick={onClose}
                        className="text-2xl hover:text-red-500 transition-colors"
                    >
                        ✕
                    </button>
                </header>

                {/* Main Content */}
                <div className="flex-1 flex flex-col items-center justify-center p-8 bg-[#FAFAFA] relative">
                    {/* Screen Lines */}
                    <div className="absolute inset-0 opacity-[0.05] pointer-events-none"
                        style={{ backgroundImage: `linear-gradient(#000 1px, transparent 1px)`, backgroundSize: '100% 4px' }}
                    />

                    {currentStepData && currentQuestion ? (
                        <div className="w-full max-w-3xl space-y-8 z-10">
                            {/* Turn Indicator */}
                            <div className="flex justify-center">
                                <span className={`px-4 py-1 text-[10px] font-bold uppercase tracking-widest border ${isMyTurn ? 'bg-[#1A1A1A] text-white border-[#1A1A1A]' : 'bg-white text-[#1A1A1A] border-[#1A1A1A]'
                                    }`}>
                                    {isMyTurn ? 'YOUR ACTION' : 'OPPONENT ACTION'}
                                </span>
                            </div>

                            {/* Question */}
                            <div className="text-center space-y-4">
                                <h3 className="text-3xl font-serif text-[#1A1A1A] leading-tight">
                                    {currentQuestion.content}
                                </h3>
                            </div>

                            {/* Answers Grid */}
                            <div className="grid grid-cols-2 gap-4">
                                {['A', 'B', 'C', 'D'].map((key, idx) => {
                                    const isSelected = currentStepData.answer === key;
                                    const isCorrect = currentQuestion.correctAnswer === key;

                                    let style = "bg-white border-gray-200 text-gray-400"; // Default inactive
                                    if (isSelected && isCorrect) style = "bg-green-100 border-green-500 text-green-800 shadow-[4px_4px_0px_0px_rgba(22,163,74,0.5)]";
                                    else if (isSelected && !isCorrect) style = "bg-red-50 border-red-500 text-red-800 shadow-[4px_4px_0px_0px_rgba(220,38,38,0.5)]";
                                    else if (!isSelected && isCorrect) style = "bg-green-50 border-green-200 text-green-600 opacity-60"; // Show correct answer if missed

                                    return (
                                        <div key={key} className={`p-4 border font-medium transition-all ${style}`}>
                                            <span className="mr-2 font-serif italic">{key}.</span>
                                            {currentQuestion.answers[idx]}
                                        </div>
                                    );
                                })}
                            </div>

                            {/* Result Text */}
                            <div className="text-center h-8">
                                {currentStepData.answer === currentQuestion.correctAnswer
                                    ? <span className="text-green-600 font-serif italic">Correct decision made.</span>
                                    : <span className="text-red-600 font-serif italic">Incorrect judgment.</span>
                                }
                            </div>
                        </div>
                    ) : (
                        <div className="text-center">
                            <h3 className="text-4xl font-serif italic mb-4">Playback Concluded</h3>
                            <div className="flex items-center justify-center gap-8 font-mono text-sm">
                                <div>SCORE: {game.score}</div>
                                <div>RANK: #{game.rank}</div>
                            </div>
                        </div>
                    )}
                </div>

                {/* Footer Controls */}
                <footer className="p-6 border-t border-[#1A1A1A] bg-white/90 backdrop-blur flex items-center gap-4">
                    <button onClick={() => { setCurrentStep(0); setIsPlaying(false); }} className="p-3 hover:bg-gray-100 rounded">
                        ⏮
                    </button>
                    <button
                        onClick={isPlaying ? () => setIsPlaying(false) : handlePlay}
                        className="flex-1 py-3 bg-[#1A1A1A] text-white font-bold uppercase tracking-wider hover:bg-ren-gold transition-colors"
                    >
                        {isPlaying ? 'Pause' : 'Play Memory'}
                    </button>
                    <button
                        onClick={() => setCurrentStep(Math.min(currentStep + 1, steps.length))}
                        disabled={currentStep >= steps.length}
                        className="p-3 hover:bg-gray-100 rounded disabled:opacity-30"
                    >
                        ⏭
                    </button>
                </footer>

                {/* Progress Bar */}
                <div className="h-1 bg-gray-100 w-full">
                    <motion.div
                        className="h-full bg-ren-gold"
                        animate={{ width: `${(currentStep / Math.max(steps.length, 1)) * 100}%` }}
                    />
                </div>
            </div>
        </motion.div>
    );
};

export default ReplayModal;
