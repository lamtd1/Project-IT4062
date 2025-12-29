import React from 'react';

const RoomListPage = ({ rooms, onJoin }) => (
    <div className="h-full flex flex-col shadow-lg border-2 border-gray-200 bg-white overflow-hidden rounded-xl">
        <div className="py-4 pb-3 border-b border-gray-100 bg-gray-50 px-6">
            <h3 className="text-lg font-bold text-black">📋 Danh sách phòng</h3>
        </div>
        <div className="flex-1 overflow-y-auto p-3 space-y-2.5">
            {rooms.length === 0 ? (
                <div className="h-full flex flex-col items-center justify-center text-gray-400">
                    <div className="text-4xl mb-4 opacity-20">🕹️</div>
                    <p className="text-sm font-medium">Chưa có phòng nào</p>
                    <p className="text-xs mt-1">Hãy tạo phòng mới để bắt đầu!</p>
                </div>
            ) : (
                rooms.map((room) => (
                    <div
                        key={room.id}
                        className="group flex justify-between items-center p-3.5 rounded-lg border-2 border-gray-200 bg-gray-50 hover:bg-white hover:border-indigo-400 hover:shadow-md transition-all cursor-pointer"
                        onClick={() => onJoin(room.id)}
                    >
                        <div className="flex-1 min-w-0">
                            <div className="flex items-center gap-2 mb-1">
                                <span className="font-bold text-gray-800 text-sm truncate group-hover:text-indigo-700 transition-colors">
                                    {room.name}
                                </span>
                                {room.count >= 4 && (
                                    <span className="bg-red-500 text-white text-[10px] px-2 py-0.5 rounded-full font-bold">
                                        Full
                                    </span>
                                )}
                            </div>
                            <div className="flex items-center gap-3 text-xs text-gray-600">
                                <span className="font-mono bg-gray-200 px-1.5 py-0.5 rounded text-gray-700">
                                    #{room.id}
                                </span>
                                <span className="flex items-center gap-1">
                                    👤 {room.count}/4
                                </span>
                            </div>
                        </div>

                        <div className="pl-3">
                            <div className={`px-2.5 py-1 rounded-full text-[10px] font-bold border-2 flex items-center gap-1.5 ${room.status === '0'
                                    ? 'bg-emerald-50 text-emerald-700 border-emerald-500'
                                    : 'bg-rose-50 text-rose-700 border-rose-500'
                                }`}>
                                <span className={`w-1.5 h-1.5 rounded-full ${room.status === '0' ? 'bg-emerald-500' : 'bg-rose-500'}`}></span>
                                {room.status === '0' ? 'Waiting' : 'Playing'}
                            </div>
                        </div>
                    </div>
                ))
            )}
        </div>
    </div>
);

export default RoomListPage;
