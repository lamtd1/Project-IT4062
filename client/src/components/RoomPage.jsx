import React from 'react';

const RoomPage = ({ roomInfo, members, isHost, onLeave, onStart }) => (
    <div className="bg-white p-6 rounded-2xl shadow-xl max-w-4xl w-full flex gap-6 min-h-[500px]">
        {/* Left: Room Info & Actions */}
        <div className="flex-1 flex flex-col">
            <div className="mb-6">
                <h2 className="text-3xl font-bold text-blue-800 mb-1">{roomInfo.name}</h2>
                <p className="text-gray-500 font-medium">Room ID: {roomInfo.id}</p>
            </div>

            <div className="flex-1 bg-gray-50 rounded-xl p-4 border border-gray-100 mb-6">
                <p className="text-gray-700 mb-4 font-medium flex items-center gap-2">
                    <span className="w-2 h-2 rounded-full bg-green-500 animate-pulse"></span>
                    Trạng thái: {isHost ? "Bạn là chủ phòng" : "Đang chờ chủ phòng..."}
                </p>

                {isHost ? (
                    <button
                        onClick={onStart}
                        disabled={members.length < 1}
                        className={`w-full py-4 rounded-xl font-bold text-lg shadow-lg transition-all transform hover:scale-105
                        ${members.length < 1
                                ? 'bg-gray-300 text-gray-500 cursor-not-allowed'
                                : 'bg-gradient-to-r from-green-400 to-green-600 text-white hover:shadow-green-500/50'}`}
                    >
                        {members.length < 1 ? 'Cần ít nhất 1 người chơi' : 'BẮT ĐẦU GAME 🚀'}
                    </button>
                ) : (
                    <div className="w-full py-4 bg-gray-200 rounded-xl text-center text-gray-500 font-medium animate-pulse">
                        Đợi chủ phòng bắt đầu...
                    </div>
                )}
            </div>

            <button onClick={onLeave} className="px-6 py-2 text-red-500 hover:bg-red-50 hover:text-red-600 rounded-lg font-bold transition self-start">
                ← Rời phòng
            </button>
        </div>

        {/* Right: Member List */}
        <div className="w-80 bg-gray-50 rounded-xl border border-gray-200 p-4 flex flex-col">
            <h3 className="font-bold text-gray-800 mb-4 flex justify-between items-center">
                Thành viên
                <span className="bg-blue-100 text-blue-700 px-2 py-0.5 rounded-full text-xs">{members.length}/4</span>
            </h3>
            <div className="space-y-2 overflow-y-auto flex-1">
                {members.map((mem, idx) => (
                    <div key={idx} className="flex items-center gap-3 p-3 bg-white rounded-lg border border-gray-100 shadow-sm">
                        <div className="w-10 h-10 rounded-full bg-gradient-to-br from-blue-400 to-blue-600 flex items-center justify-center text-white font-bold text-sm shadow">
                            {mem.username.charAt(0).toUpperCase()}
                        </div>
                        <div>
                            <p className="font-bold text-gray-800 text-sm flex items-center gap-1">
                                {mem.username}
                                {mem.isHost && <span className="text-xs text-yellow-500" title="Chủ phòng">👑</span>}
                            </p>
                            <p className="text-xs text-gray-500 font-medium">Điểm: {mem.score}</p>
                        </div>
                    </div>
                ))}
                {[...Array(4 - members.length)].map((_, idx) => (
                    <div key={`empty-${idx}`} className="flex items-center gap-3 p-3 border-2 border-dashed border-gray-200 rounded-lg opacity-50">
                        <div className="w-10 h-10 rounded-full bg-gray-200"></div>
                        <div className="h-4 w-24 bg-gray-200 rounded"></div>
                    </div>
                ))}
            </div>
        </div>
    </div>
);

export default RoomPage;
