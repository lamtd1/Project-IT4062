import React from 'react';
import { Card, CardContent, CardHeader, CardTitle } from "./ui/card";
import { Badge } from "./ui/badge";

const RoomListPage = ({ rooms, onJoin }) => (
    <Card className="h-full flex flex-col shadow-lg border-zinc-200 bg-white overflow-hidden">
        <CardHeader className="py-4 pb-3 border-b border-zinc-100 bg-zinc-50/50">
            <CardTitle className="text-base font-bold text-zinc-900">Danh sách phòng</CardTitle>
        </CardHeader>
        <CardContent className="flex-1 overflow-y-auto p-3 space-y-2.5">
            {rooms.length === 0 ? (
                <div className="h-full flex flex-col items-center justify-center text-zinc-400">
                    <div className="text-4xl mb-4 opacity-20">🕹️</div>
                    <p className="text-sm font-medium">Chưa có phòng nào</p>
                    <p className="text-xs mt-1">Hãy tạo phòng mới để bắt đầu!</p>
                </div>
            ) : (
                rooms.map((room) => (
                    <div
                        key={room.id}
                        className="group flex justify-between items-center p-3.5 rounded-lg border border-zinc-100 bg-zinc-50/50 hover:bg-white hover:border-indigo-200 hover:shadow-md transition-all cursor-pointer"
                        onClick={() => onJoin(room.id)}
                    >
                        <div className="flex-1 min-w-0">
                            <div className="flex items-center gap-2 mb-1">
                                <span className="font-bold text-zinc-800 text-sm truncate group-hover:text-indigo-700 transition-colors">
                                    {room.name}
                                </span>
                                {room.count >= 4 && <Badge variant="destructive" className="text-[10px] px-1 h-4">Full</Badge>}
                            </div>
                            <div className="flex items-center gap-3 text-xs text-zinc-500">
                                <span className="font-mono bg-zinc-100 px-1.5 py-0.5 rounded text-zinc-600">#{room.id}</span>
                                <span className="flex items-center gap-1">
                                    👤 {room.count}/4
                                </span>
                            </div>
                        </div>

                        <div className="pl-3">
                            <div className={`px-2.5 py-1 rounded-full text-[10px] font-bold border flex items-center gap-1.5 ${room.status === '0'
                                    ? 'bg-emerald-50 text-emerald-700 border-emerald-200'
                                    : 'bg-rose-50 text-rose-700 border-rose-200'
                                }`}>
                                <span className={`w-1.5 h-1.5 rounded-full ${room.status === '0' ? 'bg-emerald-500' : 'bg-rose-500'}`}></span>
                                {room.status === '0' ? 'Waiting' : 'Playing'}
                            </div>
                        </div>
                    </div>
                ))
            )}
        </CardContent>
    </Card>
);

export default RoomListPage;
