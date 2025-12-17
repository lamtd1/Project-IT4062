import React from 'react';

const RoomListPage = ({ rooms, onJoin }) => (
    <div className="flex-1 bg-white p-4 rounded-xl shadow-sm border border-gray-100 h-full flex flex-col">
        <h3 className="font-bold text-gray-800 mb-3 text-lg">Danh sách phòng</h3>
        <div className="flex-1 overflow-y-auto space-y-2 pr-1">
            {rooms.length === 0 ? (
                <div className="h-full flex flex-col items-center justify-center text-gray-400">
                    <p>Chưa có phòng nào</p>
                </div>
            ) : (
                rooms.map((room) => (
                    <div key={room.id} className="flex justify-between items-center p-3 border border-gray-200 rounded-lg hover:bg-blue-50 transition cursor-pointer" onClick={() => onJoin(room.id)}>
                        <div>
                            <div className="font-bold text-gray-800">{room.name}</div>
                            <div className="text-xs text-gray-500">ID: {room.id}</div>
                        </div>
                        <div className="flex items-center gap-2">
                            <span className="text-xs font-semibold bg-gray-100 px-2 py-1 rounded text-gray-600">
                                {room.count}/4
                            </span>
                            <span className={`w-2 h-2 rounded-full ${room.status === '0' ? 'bg-green-500' : 'bg-red-500'}`}></span>
                        </div>
                    </div>
                ))
            )}
        </div>
    </div>
);

export default RoomListPage;
