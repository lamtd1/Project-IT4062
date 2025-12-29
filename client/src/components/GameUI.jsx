import React from 'react';
import { Card, CardContent } from "./ui/card";
import { Button } from "./ui/button";

const GameUI = ({ currentQuestion, timeLeft, handleAnswer, socket }) => (
    <div className="min-h-screen w-full flex items-center justify-center p-6 bg-zinc-950 text-zinc-50">
        <div className="max-w-7xl w-full grid grid-cols-12 gap-6">
            {/* Main Content Area - Left Side */}
            <div className="col-span-9 flex flex-col gap-6">
                {/* Question Display */}
                <Card className="bg-zinc-900 border-zinc-800 shadow-xl min-h-[300px] flex items-center justify-center relative overflow-hidden">
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
                            disabled={currentQuestion?.canAnswer === false}
                            className={`h-auto p-6 text-lg justify-start gap-4 transition-all group ${
                                currentQuestion?.canAnswer === false 
                                    ? 'bg-zinc-950 border-zinc-900 opacity-50 cursor-not-allowed' 
                                    : 'bg-zinc-900 hover:bg-zinc-800 border border-zinc-800 hover:border-zinc-700'
                            }`}
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
