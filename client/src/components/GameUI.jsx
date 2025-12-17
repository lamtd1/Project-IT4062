import React from 'react';

const GameUI = ({ currentQuestion, timeLeft, handleAnswer, socket }) => (
    <div className="bg-gradient-to-br from-blue-900 via-purple-900 to-indigo-900 min-h-screen w-full flex items-center justify-center p-6">
        <div className="max-w-7xl w-full grid grid-cols-12 gap-6">
            {/* Main Content Area - Left Side */}
            <div className="col-span-9 flex flex-col gap-6">
                {/* Question Display */}
                <div className="bg-gradient-to-br from-blue-600 to-purple-600 rounded-3xl p-8 shadow-2xl border-4 border-yellow-400 min-h-[300px] flex items-center justify-center">
                    <h2 className="text-3xl font-bold text-white text-center leading-relaxed drop-shadow-lg">
                        {currentQuestion?.content || "Đang tải câu hỏi..."}
                    </h2>
                </div>

                {/* Answer Buttons */}
                <div className="grid grid-cols-2 gap-4">
                    {['A', 'B', 'C', 'D'].map((char, idx) => (
                        <button
                            key={char}
                            onClick={() => handleAnswer(char)}
                            className="group relative p-6 rounded-2xl bg-gradient-to-r from-blue-500 to-blue-600 hover:from-yellow-400 hover:to-yellow-500 text-white hover:text-gray-900 font-bold text-lg shadow-xl transition-all duration-300 transform hover:scale-105 hover:shadow-2xl border-4 border-transparent hover:border-yellow-300"
                        >
                            <div className="flex items-center gap-4">
                                <div className="w-12 h-12 rounded-full bg-white/20 group-hover:bg-gray-900/20 flex items-center justify-center text-2xl font-black">
                                    {char}
                                </div>
                                <span className="flex-1 text-left text-lg">
                                    {currentQuestion?.answers[idx] || `Đáp án ${char}`}
                                </span>
                            </div>
                        </button>
                    ))}
                </div>
            </div>

            {/* Right Sidebar */}
            <div className="col-span-3 flex flex-col gap-6">
                {/* Timer & Question Progress */}
                <div className="bg-white/10 backdrop-blur-lg rounded-2xl p-6 border-2 border-white/20">
                    <div className="text-center mb-4">
                        <div className="text-6xl font-black text-yellow-400 mb-2 drop-shadow-lg animate-pulse">
                            {timeLeft}s
                        </div>
                        <div className="text-white/80 text-sm font-medium">Thời gian còn lại</div>
                    </div>

                    <div className="h-px bg-white/20 my-4"></div>

                    <div className="text-center">
                        <div className="text-4xl font-black text-white mb-2">
                            {currentQuestion?.id || 1}<span className="text-white/50 text-2xl">/15</span>
                        </div>
                        <div className="text-white/80 text-sm font-medium">Câu hỏi</div>
                    </div>
                </div>

                {/* Lifelines */}
                <div className="bg-white/10 backdrop-blur-lg rounded-2xl p-6 border-2 border-white/20">
                    <h3 className="text-white font-bold text-lg mb-4 text-center">Quyền Trợ Giúp</h3>
                    <div className="space-y-3">
                        {/* 50-50 */}
                        <button
                            className="w-full bg-gradient-to-r from-green-500 to-green-600 hover:from-green-600 hover:to-green-700 text-white font-bold py-3 px-4 rounded-xl shadow-lg transition-all transform hover:scale-105"
                            onClick={() => {
                                // TODO: Send lifeline request
                                console.log("50-50 requested");
                            }}
                        >
                            <div className="flex items-center gap-2">
                                <span className="text-2xl">⚖️</span>
                                <span className="text-sm">50-50</span>
                            </div>
                        </button>

                        {/* Hỏi khán giả */}
                        <button
                            className="w-full bg-gradient-to-r from-blue-500 to-blue-600 hover:from-blue-600 hover:to-blue-700 text-white font-bold py-3 px-4 rounded-xl shadow-lg transition-all transform hover:scale-105"
                            onClick={() => {
                                console.log("Audience poll requested");
                            }}
                        >
                            <div className="flex items-center gap-2">
                                <span className="text-2xl">👥</span>
                                <span className="text-sm">Khán giả</span>
                            </div>
                        </button>

                        {/* Gọi điện */}
                        <button
                            className="w-full bg-gradient-to-r from-purple-500 to-purple-600 hover:from-purple-600 hover:to-purple-700 text-white font-bold py-3 px-4 rounded-xl shadow-lg transition-all transform hover:scale-105"
                            onClick={() => {
                                console.log("Phone a friend requested");
                            }}
                        >
                            <div className="flex items-center gap-2">
                                <span className="text-2xl">📞</span>
                                <span className="text-sm">Gọi điện</span>
                            </div>
                        </button>

                        {/* Chuyên gia */}
                        <button
                            className="w-full bg-gradient-to-r from-orange-500 to-orange-600 hover:from-orange-600 hover:to-orange-700 text-white font-bold py-3 px-4 rounded-xl shadow-lg transition-all transform hover:scale-105"
                            onClick={() => {
                                console.log("Expert advice requested");
                            }}
                        >
                            <div className="flex items-center gap-2">
                                <span className="text-2xl">🎓</span>
                                <span className="text-sm">Chuyên gia</span>
                            </div>
                        </button>
                    </div>
                </div>

                {/* Walk Away Button */}
                <button
                    onClick={() => {
                        const buf = new Uint8Array(1);
                        buf[0] = 0x2B; // MSG_WALK_AWAY
                        socket.emit('client_to_server', buf);
                    }}
                    className="bg-gradient-to-r from-red-500 to-red-600 hover:from-red-600 hover:to-red-700 text-white font-bold py-4 px-6 rounded-xl shadow-xl transition-all transform hover:scale-105"
                >
                    <div className="flex items-center justify-center gap-2">
                        <span className="text-2xl">🚪</span>
                        <span>Dừng cuộc chơi</span>
                    </div>
                </button>
            </div>
        </div>
    </div>
);

export default GameUI;
