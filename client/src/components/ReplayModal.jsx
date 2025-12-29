import React, { useState, useEffect } from 'react';
import { OPS } from '../lib/ops';

const ReplayModal = ({ game, userId, socket, onClose }) => {
    const [currentStep, setCurrentStep] = useState(0);
    const [isPlaying, setIsPlaying] = useState(false);
    const [steps, setSteps] = useState([]);
    const [questions, setQuestions] = useState({});
    const [questionsLoaded, setQuestionsLoaded] = useState(false);

    // Fetch questions when modal opens
    useEffect(() => {
        if (!game.logData) {
            setSteps([]);
            setQuestionsLoaded(true);
            return;
        }

        // Parse log data: userId:questionId:answer,userId:questionId:answer...
        const entries = game.logData.split(',').filter(e => e);
        const parsedSteps = entries.map(entry => {
            const [playerId, questionId, answer] = entry.split(':');
            return {
                userId: parseInt(playerId),
                questionId: parseInt(questionId),
                answer
            };
        });

        setSteps(parsedSteps);
        console.log('[REPLAY DEBUG] Parsed steps:', parsedSteps.length, parsedSteps.slice(0, 3));

        // Get unique question IDs
        const questionIds = [...new Set(parsedSteps.map(s => s.questionId))];
        console.log('[REPLAY DEBUG] Question IDs to fetch:', questionIds);
        if (questionIds.length === 0) {
            setQuestionsLoaded(true);
            return;
        }

        // Request questions from server
        const idsString = questionIds.join(',');
        console.log('[REPLAY DEBUG] Requesting questions for IDs:', idsString);
        const encoder = new TextEncoder();
        const idsBytes = encoder.encode(idsString);
        const packet = new Uint8Array(1 + idsBytes.length);
        packet[0] = OPS.GET_QUESTIONS_BY_IDS;
        packet.set(idsBytes, 1);

        console.log('[REPLAY DEBUG] Emitting packet, opcode:', OPS.GET_QUESTIONS_BY_IDS, 'size:', packet.length);
        socket.emit('client_to_server', packet);

        // Listen for response
        const handleServerMessage = (data) => {
            console.log('[REPLAY DEBUG] Received server message, checking opcode...');
            const view = new Uint8Array(data);
            const opcode = view[0];
            console.log('[REPLAY DEBUG] Opcode:', opcode, 'Expected:', OPS.QUESTIONS_RESPONSE);

            if (opcode === OPS.QUESTIONS_RESPONSE) {
                const textDecoder = new TextDecoder();
                const payload = textDecoder.decode(view.slice(1));

                const questionsMap = {};
                if (payload) {
                    const questionEntries = payload.split(';').filter(e => e);
                    console.log('[REPLAY DEBUG] Question entries:', questionEntries);
                    questionEntries.forEach(entry => {
                        const [id, content, ansA, ansB, ansC, ansD, correct] = entry.split('|');
                        console.log('[REPLAY DEBUG] Parsing question:', { id, content: content?.substring(0, 30) });
                        questionsMap[parseInt(id)] = {
                            id: parseInt(id),
                            content,
                            answers: [ansA, ansB, ansC, ansD],
                            correctAnswer: correct
                        };
                    });
                }

                console.log('[REPLAY DEBUG] Questions map:', Object.keys(questionsMap));
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

        const timer = setTimeout(() => {
            setCurrentStep(prev => prev + 1);
        }, 2000);

        return () => clearTimeout(timer);
    }, [isPlaying, currentStep, steps.length]);

    const handlePlay = () => {
        if (currentStep >= steps.length) {
            setCurrentStep(0);
        }
        setIsPlaying(true);
    };

    const handlePause = () => setIsPlaying(false);
    const handleReset = () => {
        setCurrentStep(0);
        setIsPlaying(false);
    };

    const currentStepData = steps[currentStep];
    const isMyTurn = currentStepData?.userId === userId;
    const currentQuestion = currentStepData ? questions[currentStepData.questionId] : null;

    console.log('[REPLAY DEBUG] Render:', {
        currentStep,
        stepUserId: currentStepData?.userId,
        stepQuestionId: currentStepData?.questionId,
        hasQuestion: !!currentQuestion,
        questionsKeys: Object.keys(questions)
    });

    const getAnswerStyle = (choice) => {
        if (!currentStepData || !currentQuestion) return 'bg-zinc-900 border-zinc-800 text-zinc-300';

        const isSelected = currentStepData.answer === choice;
        const isCorrect = currentQuestion.correctAnswer === choice;

        if (!isSelected) {
            return 'bg-zinc-900 border-zinc-800 hover:bg-zinc-800 text-zinc-300';
        }

        if (isCorrect) {
            return 'bg-green-600 border-green-500 text-white shadow-lg shadow-green-500/50';
        } else {
            return 'bg-red-600 border-red-500 text-white shadow-lg shadow-red-500/50';
        }
    };

    // Loading state
    if (!questionsLoaded) {
        return (
            <div className="fixed inset-0 bg-black/80 flex items-center justify-center z-50">
                <div className="bg-zinc-900 border-2 border-zinc-800 rounded-xl p-8 text-center">
                    <div className="w-16 h-16 border-4 border-zinc-700 border-t-indigo-500 rounded-full animate-spin mx-auto mb-4"></div>
                    <p className="text-white">Loading replay...</p>
                </div>
            </div>
        );
    }

    // Render complete modal when loaded
    return (
        <div
            className="fixed inset-0 bg-black/80 flex items-center justify-center z-50 p-4"
            onClick={onClose}
        >
            <div
                className="bg-zinc-950 border-2 border-zinc-800 rounded-2xl w-full max-w-6xl max-h-[95vh] overflow-hidden shadow-2xl flex flex-col"
                onClick={(e) => e.stopPropagation()}
            >
                {/* Header */}
                <div className="flex justify-between items-center p-4 border-b border-zinc-800 bg-zinc-900/50">
                    <div>
                        <h2 className="text-xl font-bold text-white">📹 {game.roomName}</h2>
                        <p className="text-sm text-zinc-400">
                            Step {currentStep + 1} / {steps.length} •
                            {game.gameMode === 1 ? ' ⚔️ Elimination' : ' ⚡ Score Attack'}
                        </p>
                    </div>
                    <button
                        onClick={onClose}
                        className="w-10 h-10 bg-zinc-800 border border-zinc-700 rounded-lg text-zinc-300 text-xl hover:bg-red-600 hover:border-red-500 hover:text-white transition-all"
                    >
                        ✕
                    </button>
                </div>

                {/* Game Content */}
                <div className="flex-1 overflow-y-auto p-6 bg-zinc-950">
                    {currentStepData && currentQuestion ? (
                        <div className="max-w-5xl mx-auto">
                            {/* Player Indicator */}
                            <div className="mb-6 text-center">
                                <div className={`inline-flex items-center gap-2 px-4 py-2 rounded-lg border-2 ${isMyTurn
                                    ? 'bg-indigo-500/20 border-indigo-500 text-indigo-400'
                                    : 'bg-amber-500/20 border-amber-500 text-amber-400'
                                    }`}>
                                    <span className="text-lg">{isMyTurn ? '👤 Your Turn' : '🎭 Opponent\'s Turn'}</span>
                                </div>
                            </div>

                            {/* Question Display */}
                            <div className="bg-zinc-900 border border-zinc-800 rounded-xl p-8 mb-6 min-h-[200px] flex items-center justify-center relative">
                                <div className="absolute top-0 left-0 right-0 h-1 bg-gradient-to-r from-indigo-500 via-purple-500 to-pink-500"></div>
                                <h3 className="text-2xl font-bold text-center text-zinc-100">
                                    {currentQuestion.content}
                                </h3>
                            </div>

                            {/* Answer Buttons */}
                            <div className="grid grid-cols-2 gap-4 mb-6">
                                {['A', 'B', 'C', 'D'].map((choice, idx) => (
                                    <div
                                        key={choice}
                                        className={`p-4 rounded-xl border-2 flex items-center gap-4 transition-all ${getAnswerStyle(choice)}`}
                                    >
                                        <span className="flex-shrink-0 w-10 h-10 flex items-center justify-center rounded-full bg-zinc-800 text-zinc-400 font-bold text-sm">
                                            {choice}
                                        </span>
                                        <span className="flex-1 font-medium">
                                            {currentQuestion.answers[idx]}
                                        </span>
                                        {currentStepData.answer === choice && (
                                            <span className="flex-shrink-0 text-2xl">
                                                {currentQuestion.correctAnswer === choice ? '✓' : '✗'}
                                            </span>
                                        )}
                                    </div>
                                ))}
                            </div>

                            {/* Result Indicator */}
                            <div className="text-center">
                                {currentStepData.answer === currentQuestion.correctAnswer ? (
                                    <p className="text-green-400 font-bold text-lg">✓ Correct!</p>
                                ) : (
                                    <p className="text-red-400 font-bold text-lg">✗ Wrong answer - Correct: {currentQuestion.correctAnswer}</p>
                                )}
                            </div>
                        </div>
                    ) : (
                        <div className="h-full flex items-center justify-center text-center">
                            <div>
                                <h3 className="text-3xl font-bold mb-4 text-white">🎬 Replay Complete</h3>
                                <p className="text-xl mb-2 text-zinc-300">
                                    Final Score: <strong className="text-indigo-400 text-3xl">{game.score}</strong>
                                </p>
                                <p className="text-zinc-400 mb-1">Rank: #{game.rank}</p>
                                {game.winnerId === userId && (
                                    <p className="text-yellow-400 text-2xl font-bold mt-6 drop-shadow-lg animate-pulse">
                                        🏆 You Won!
                                    </p>
                                )}
                            </div>
                        </div>
                    )}
                </div>

                {/* Controls */}
                <div className="border-t border-zinc-800 bg-zinc-900/50 p-4">
                    <div className="flex gap-3 max-w-2xl mx-auto">
                        <button
                            onClick={handleReset}
                            className="flex-1 py-3 bg-zinc-800 border border-zinc-700 rounded-lg text-zinc-300 font-semibold hover:bg-zinc-700 transition-all"
                        >
                            ⏮️ Reset
                        </button>
                        <button
                            onClick={isPlaying ? handlePause : handlePlay}
                            className="flex-1 py-3 bg-indigo-600 text-white font-semibold rounded-lg border border-indigo-500 hover:bg-indigo-700 transition-all"
                        >
                            {isPlaying ? '⏸️ Pause' : '▶️ Play'}
                        </button>
                        <button
                            onClick={() => setCurrentStep(Math.min(currentStep + 1, steps.length))}
                            disabled={currentStep >= steps.length}
                            className="flex-1 py-3 bg-zinc-800 border border-zinc-700 rounded-lg text-zinc-300 font-semibold hover:bg-zinc-700 transition-all disabled:opacity-30 disabled:cursor-not-allowed disabled:hover:bg-zinc-800"
                        >
                            ⏭️ Next
                        </button>
                    </div>

                    {/* Progress Bar */}
                    <div className="mt-4 h-2 bg-zinc-800 rounded-full overflow-hidden">
                        <div
                            className="h-full bg-indigo-600 transition-all duration-300"
                            style={{ width: `${(currentStep / Math.max(steps.length, 1)) * 100}%` }}
                        />
                    </div>
                </div>
            </div>
        </div>
    );
};

export default ReplayModal;
