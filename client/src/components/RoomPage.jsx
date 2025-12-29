import React, { useState } from 'react';

const RoomPage = ({ roomInfo, members, isHost, onLeave, onStart, idleUsers = [], onGetIdleUsers, onSendInvite }) => {
    const [showInvite, setShowInvite] = useState(false);

    const isMode0 = roomInfo.name && roomInfo.name.includes(':0');
    const maxPlayers = isMode0 ? 1 : 4;

    const handleOpenInvite = () => {
        if (onGetIdleUsers) onGetIdleUsers();
        setShowInvite(true);
    };

    const handleInviteClick = (user) => {
        if (onSendInvite) onSendInvite(user);
    };

    return (
        <React.Fragment>
            {/* Invitation Modal */}
            {showInvite && (
                <div className="fixed inset-0 bg-black/50 backdrop-blur-sm z-50 flex items-center justify-center">
                    <div className="w-full max-w-md bg-white shadow-xl border-2 border-gray-200 rounded-xl">
                        <div className="flex flex-row items-center justify-between p-6">
                            <h3 className="text-xl font-semibold text-black">Mời bạn bè</h3>
                            <button onClick={() => setShowInvite(false)} className="px-3 py-1 hover:bg-gray-100 rounded transition">✕</button>
                        </div>
                        <div className="space-y-4 px-6 pb-6">
                            <div className="flex justify-between items-center bg-gray-50 p-2 rounded text-xs text-gray-600">
                                <span>Người chơi đang rảnh: {idleUsers.length}</span>
                                <button onClick={onGetIdleUsers} className="text-indigo-600 hover:underline">Làm mới</button>
                            </div>
                            <div className="max-h-[300px] overflow-y-auto space-y-2">
                                {idleUsers.length === 0 ? (
                                    <div className="text-center py-8 text-gray-400">Không có ai đang rảnh :(</div>
                                ) : (
                                    idleUsers.map((u, idx) => (
                                        <div key={idx} className="flex justify-between items-center p-3 rounded-lg border-2 border-gray-200 hover:bg-gray-50 transition-colors">
                                            <div className="flex items-center gap-3">
                                                <div className="w-8 h-8 rounded-full bg-indigo-100 text-indigo-600 flex items-center justify-center text-xs font-bold">
                                                    {u.charAt(0).toUpperCase()}
                                                </div>
                                                <span className="font-semibold text-gray-700">{u}</span>
                                            </div>
                                            <button onClick={() => handleInviteClick(u)} className="h-8 px-4 py-1 border-2 border-indigo-500 text-indigo-600 hover:bg-indigo-50 rounded-lg transition">
                                                Mời
                                            </button>
                                        </div>
                                    ))
                                )}
                            </div>
                        </div>
                    </div>
                </div>
            )}

            <div className="max-w-5xl w-full h-[650px] flex shadow-2xl border-2 border-gray-200 bg-white rounded-xl">
                <div className="flex-1 p-8 flex flex-col border-r border-gray-200">
                    <div className="mb-8 flex justify-between items-start">
                        <div>
                            <h2 className="text-3xl font-bold text-black tracking-tight mb-2">{roomInfo.name}</h2>
                            <div className="flex items-center gap-2">
                                <span className="px-2 py-1 rounded-md bg-gray-100 text-gray-700 font-mono text-xs border-2 border-gray-300">
                                    Room ID: {roomInfo.id}
                                </span>
                                <span className={`px-3 py-1 rounded-full text-xs font-bold ${isHost ? 'bg-indigo-600 text-white' : 'bg-gray-200 text-gray-700'}`}>
                                    {isHost ? "Host" : "Member"}
                                </span>
                            </div>
                        </div>
                    </div>

                    <div className="flex-1 mb-6 bg-gray-50 border-2 border-gray-200 shadow-inner flex items-center justify-center p-6 relative overflow-hidden rounded-xl">
                        <div className="absolute inset-0 opacity-5" />

                        {isHost ? (
                            <div className="text-center w-full max-w-sm relative z-10">
                                <button
                                    onClick={onStart}
                                    disabled={members.length < 1}
                                    className={`w-full py-8 text-xl font-bold transition-all rounded-xl ${members.length < 1 ? 'opacity-50 bg-gray-400 cursor-not-allowed' : 'hover:scale-105 shadow-xl bg-indigo-600 hover:bg-indigo-700 text-white'
                                        }`}
                                >
                                    {members.length < 1 ? 'Chờ người chơi...' : 'BẮT ĐẦU GAME 🚀'}
                                </button>
                                <p className="mt-4 text-gray-600 text-sm font-medium">
                                    {members.length < 1 ? "Cần ít nhất 1 người chơi để bắt đầu" : "Đã sẵn sàng!"}
                                </p>
                            </div>
                        ) : (
                            <div className="text-center space-y-4 relative z-10">
                                <div className="w-16 h-16 border-4 border-gray-200 border-t-indigo-600 rounded-full animate-spin mx-auto"></div>
                                <p className="text-gray-600 font-medium">Đang chờ chủ phòng bắt đầu...</p>
                            </div>
                        )}
                    </div>

                    <button onClick={onLeave} className="self-start px-4 py-2 text-red-600 hover:text-red-700 hover:bg-red-50 border-2 border-red-300 rounded-lg transition">
                        ← Rời phòng
                    </button>
                </div>

                {/* Sidebar */}
                <div className="w-80 bg-gray-50 p-6 flex flex-col border-l border-gray-200">
                    <div className="flex justify-between items-center mb-6">
                        <h3 className="font-semibold text-black flex items-center gap-2">
                            Người chơi
                            <span className="bg-white border-2 border-gray-300 px-2 py-0.5 rounded text-black text-sm">{members.length}/{maxPlayers}</span>
                        </h3>
                        {isHost && !isMode0 && (
                            <button onClick={handleOpenInvite} className="h-8 px-2 text-indigo-600 hover:text-indigo-700 hover:bg-indigo-50 rounded transition">
                                + Mời
                            </button>
                        )}
                    </div>

                    <div className="space-y-3 flex-1 overflow-y-auto">
                        {members.map((mem, idx) => (
                            <div key={idx} className="p-3 flex items-center gap-3 border-2 border-gray-200 shadow-sm transition-all hover:shadow-md bg-white rounded-lg">
                                <div className={`w-10 h-10 rounded-full flex items-center justify-center text-white font-bold text-sm shadow-sm ${mem.isHost ? 'bg-indigo-600' : 'bg-gray-400'
                                    }`}>
                                    {mem.username.charAt(0).toUpperCase()}
                                </div>
                                <div className="flex-1 truncate">
                                    <div className="flex items-center gap-1.5">
                                        <span className="font-semibold text-sm text-black truncate">{mem.username}</span>
                                        {mem.isHost && <span className="text-[10px] bg-indigo-50 text-indigo-700 px-1.5 py-0.5 rounded border border-indigo-200 font-bold">HOST</span>}
                                    </div>
                                    <div className="text-xs text-gray-600">Điểm: {mem.score}</div>
                                </div>
                            </div>
                        ))}
                        {!isMode0 && Array.from({ length: Math.max(0, maxPlayers - members.length) }).map((_, i) => (
                            <div key={`empty-${i}`} className="p-3 border-2 border-dashed border-gray-300 rounded-lg flex items-center gap-3 opacity-50">
                                <div className="w-10 h-10 rounded-full bg-gray-200"></div>
                                <div className="h-4 bg-gray-200 w-20 rounded"></div>
                            </div>
                        ))}
                    </div>
                </div>
            </div>
        </React.Fragment>
    );
};

export default RoomPage;
