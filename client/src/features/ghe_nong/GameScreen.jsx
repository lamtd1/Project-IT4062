import { useState } from 'react';

export default function GameScreen() {
    const [selectedAnswer, setSelectedAnswer] = useState(null);

    const question = {
        text: "Which planet is known as the Red Planet?",
        answers: [
            "Venus",
            "Mars",
            "Jupiter",
            "Saturn"
        ],
        correct: 1
    };

    const moneyLadder = [
        "100", "200", "300", "500", "1,000",
        "2,000", "4,000", "8,000", "16,000", "32,000",
        "64,000", "125,000", "250,000", "500,000", "1 MILLION"
    ].reverse();

    return (
        <div className="h-screen flex flex-col bg-gradient-to-b from-blue-900 to-black p-4">
            {/* Top Bar */}
            <div className="flex justify-between items-center mb-4">
                <div className="flex gap-2">
                    {['50:50', '📞', '👥'].map((lifeline, idx) => (
                        <button
                            key={idx}
                            className="w-16 h-10 bg-blue-900 border-2 border-blue-400 rounded-full flex items-center justify-center hover:bg-blue-800 transition-colors shadow-[0_0_10px_rgba(59,130,246,0.5)]"
                        >
                            {lifeline}
                        </button>
                    ))}
                </div>
                <div className="text-right">
                    <div className="text-yellow-400 font-bold text-xl">Level 5</div>
                    <div className="text-white text-sm">Prize: $1,000</div>
                </div>
            </div>

            <div className="flex-1 flex gap-4">
                {/* Main Game Area */}
                <div className="flex-1 flex flex-col justify-end pb-8">
                    {/* Question */}
                    <div className="mb-8 relative">
                        <div className="bg-blue-900/80 border-2 border-white/30 p-8 rounded-xl text-center shadow-[0_0_20px_rgba(0,0,0,0.5)]">
                            <h2 className="text-2xl md:text-3xl font-bold text-white leading-relaxed">
                                {question.text}
                            </h2>
                        </div>
                        {/* Decorative lines connecting question to answers could go here */}
                    </div>

                    {/* Answers */}
                    <div className="grid grid-cols-2 gap-4">
                        {question.answers.map((answer, idx) => (
                            <button
                                key={idx}
                                onClick={() => setSelectedAnswer(idx)}
                                className={`
                  relative p-4 rounded-full border-2 transition-all duration-300 group
                  ${selectedAnswer === idx
                                        ? 'bg-yellow-600 border-yellow-300 shadow-[0_0_15px_rgba(234,179,8,0.5)]'
                                        : 'bg-blue-900/80 border-blue-400 hover:bg-blue-800'
                                    }
                `}
                            >
                                <div className="flex items-center">
                                    <span className="text-yellow-400 font-bold mr-4 text-xl">
                                        {String.fromCharCode(65 + idx)}:
                                    </span>
                                    <span className="text-white text-lg font-medium group-hover:text-yellow-100">
                                        {answer}
                                    </span>
                                </div>
                            </button>
                        ))}
                    </div>
                </div>

                {/* Money Ladder Sidebar */}
                <div className="w-48 hidden md:block">
                    <div className="bg-blue-900/50 border border-blue-500/30 rounded-lg p-2 h-full flex flex-col justify-center gap-1">
                        {moneyLadder.map((amount, idx) => {
                            const isCurrent = idx === 10; // Mock current level
                            const isMilestone = amount.includes("000") && (amount.startsWith("1,") || amount.startsWith("32,") || amount.includes("MILLION"));

                            return (
                                <div
                                    key={idx}
                                    className={`
                    flex justify-between px-3 py-1 rounded
                    ${isCurrent ? 'bg-yellow-600 text-black font-bold shadow-lg scale-105' : 'text-yellow-500'}
                    ${isMilestone ? 'text-white' : ''}
                  `}
                                >
                                    <span className="text-xs opacity-70">{15 - idx}</span>
                                    <span>${amount}</span>
                                </div>
                            );
                        })}
                    </div>
                </div>
            </div>
        </div>
    );
}
