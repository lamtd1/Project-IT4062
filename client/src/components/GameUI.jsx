import React from 'react';
import { Card, CardContent } from "./ui/card";
import { Button } from "./ui/button";

const GameUI = ({ currentQuestion, timeLeft, handleAnswer, socket, players = [], myUsername, wrongAnswer = false }) => (
    <div className="min-h-screen w-full flex flex-col p-6 bg-zinc-950 text-zinc-50">
        {/* Score Header */}
        {players.length > 0 && (
            <div className="w-full max-w-7xl mx-auto mb-6">
                <div className="bg-zinc-900/80 backdrop-blur border border-zinc-800 rounded-2xl p-4 shadow-xl">
                    <div className="flex flex-wrap gap-4 items-center">
                        <div className="mr-auto flex items-center gap-2 text-zinc-400 font-bold uppercase tracking-wider text-sm px-2">
                            <span>🏆</span>
                            <span>Bảng điểm</span>
                        </div>
                        {players.map((p, idx) => (
                            <div
                                key={idx}
                                className={`flex items-center gap-3 px-4 py-2 rounded-xl border transition-all ${p.username === myUsername
                                    ? 'bg-indigo-500/10 border-indigo-500/50 shadow-[0_0_15px_rgba(99,102,241,0.2)]'
                                    : 'bg-zinc-800/50 border-zinc-700/50'
                                    }`}
                            >
                                <div className={`w-8 h-8 rounded-lg flex items-center justify-center font-bold text-sm shadow-inner ${p.username === myUsername
                                    ? 'bg-gradient-to-br from-indigo-500 to-purple-600 text-white'
                                    : 'bg-zinc-700 text-zinc-300'
                                    }`}>
                                    {p.username.charAt(0).toUpperCase()}
                                </div>
                                <div className="flex flex-col leading-tight">
                                    <span className={`text-sm font-bold ${p.username === myUsername ? 'text-indigo-400' : 'text-zinc-300'
                                        }`}>
                                        {p.username} {p.username === myUsername && '(Bạn)'}
                                    </span>
                                    <span className="text-xs font-mono text-zinc-400 font-medium">
                                        {p.score.toLocaleString()} pts
                                    </span>
                                </div>
                            </div>
                        ))}
                    </div>
                </div>
            </div>
        )}

        <div className="max-w-7xl w-full mx-auto grid grid-cols-12 gap-6">
            {/* Main Content Area - Left Side */}
            <div className="col-span-9 flex flex-col gap-6">
                {/* Question Display - Dimmed if wrong answer (Mode 2) */}
                <Card className={`bg-zinc-900 border-zinc-800 shadow-xl min-h-[300px] flex items-center justify-center relative overflow-hidden transition-all duration-500 ${wrongAnswer ? 'opacity-50 grayscale' : ''
                    }`}>
                    <div className="absolute top-0 left-0 w-full h-1 bg-gradient-to-r from-indigo-500 via-purple-500 to-pink-500"></div>
                    <CardContent className="p-12">
                        <h2 className="text-3xl font-bold text-center leading-relaxed text-zinc-100">
                            {currentQuestion?.content || "Loading question..."}
                        </h2>
                    </CardContent>
                </Card>

                {/* Answer Buttons */}
                <div className="grid grid-cols-2 gap-4">
                    {['A', 'B', 'C', 'D'].map((char, idx) => (
                        <Button
                            key={char}
                            onClick={() => handleAnswer(char)}
                            className="h-auto p-6 text-lg justify-start gap-4 bg-zinc-900 hover:bg-zinc-800 border border-zinc-800 hover:border-zinc-700 transition-all group"
                        >
                            <span className="flex h-10 w-10 items-center justify-center rounded-full bg-zinc-800 text-zinc-400 font-bold group-hover:bg-zinc-700 group-hover:text-white transition-colors">
                                {char}
                            </span>
                            <span className="flex-1 text-left font-medium text-zinc-300 group-hover:text-white">
                                {currentQuestion?.answers[idx] || `Answer ${char}`}
                            </span>
                        </Button>
                    ))}
                </div>
            </div>

            {/* Right Sidebar */}
            <div className="col-span-3 flex flex-col gap-6">
                {/* Timer & Question Progress */}
                <Card className="bg-zinc-900 border-zinc-800 text-center">
                    <CardContent className="p-6">
                        <div className="mb-6">
                            <div className="text-6xl font-black text-indigo-500 mb-2 font-mono tabular-nums">
                                {timeLeft}
                            </div>
                            <div className="text-zinc-500 text-xs uppercase tracking-widest font-bold">Seconds Left</div>
                        </div>
                        <div className="h-px bg-zinc-800 w-full my-4"></div>
                        <div>
                            <div className="text-3xl font-bold text-white mb-1">
                                {currentQuestion?.id || 1}<span className="text-zinc-600 text-lg">/15</span>
                            </div>
                            <div className="text-zinc-500 text-xs uppercase tracking-widest font-bold">Question</div>
                        </div>
                    </CardContent>
                </Card>

                {/* Lifelines */}
                <Card className="bg-zinc-900 border-zinc-800">
                    <CardContent className="p-4 space-y-3">
                        <div className="text-xs font-bold text-zinc-500 uppercase tracking-widest mb-2 text-center">Lifelines</div>
                        {[
                            { id: '1', icon: '⚖️', label: '50-50' },
                            { id: '2', icon: '👥', label: 'Ask Audience' },
                            { id: '3', icon: '📞', label: 'Call Friend' },
                            { id: '4', icon: '🎓', label: 'Consult Expert' },
                        ].map((help) => (
                            <Button
                                key={help.id}
                                variant="outline"
                                className="w-full justify-start gap-3 bg-zinc-950 border-zinc-800 hover:bg-zinc-800 hover:text-white text-zinc-400"
                                onClick={() => {
                                    const buf = new Uint8Array(2);
                                    buf[0] = 0x2C;
                                    buf[1] = 0x30 + parseInt(help.id);
                                    socket.emit('client_to_server', buf);
                                }}
                            >
                                <span>{help.icon}</span>
                                <span>{help.label}</span>
                            </Button>
                        ))}
                    </CardContent>
                </Card>

                {/* Walk Away Button */}
                <Button
                    variant="destructive"
                    className="w-full py-6 font-bold text-md"
                    onClick={() => {
                        const buf = new Uint8Array(1);
                        buf[0] = 0x2B; // MSG_WALK_AWAY
                        socket.emit('client_to_server', buf);
                    }}
                >
                    🚪 Walk Away
                </Button>
            </div>
        </div>
    </div>
);

export default GameUI;
