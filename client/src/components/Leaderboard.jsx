import React from 'react';
import { Card, CardContent, CardHeader, CardTitle } from "./ui/card";
import { Button } from "./ui/button";

const Leaderboard = ({ data, onRefresh }) => (
    <Card className="shadow-lg border-zinc-200 bg-white overflow-hidden flex flex-col h-full">
        <CardHeader className="py-4 pb-3 border-b border-zinc-100 bg-zinc-50/50">
            <div className="flex items-center justify-between">
                <CardTitle className="text-base font-bold flex items-center gap-2 text-zinc-900">
                    <span className="text-xl">🏆</span> Bảng Xếp Hạng
                </CardTitle>

            </div>
        </CardHeader>
        <CardContent className="space-y-1 p-2 flex-1 overflow-y-auto">
            {data.length === 0 ? (
                <div className="text-center py-10 text-zinc-400 text-sm italic">Chưa có dữ liệu xếp hạng</div>
            ) : (
                data.map((user, idx) => (
                    <div key={idx} className="flex justify-between items-center p-2.5 rounded-md hover:bg-zinc-50 transition-colors cursor-default group">
                        <div className="flex items-center gap-3">
                            <div className={`flex items-center justify-center w-6 h-6 rounded-full text-xs font-bold ${idx === 0 ? 'bg-yellow-100 text-yellow-700' :
                                idx === 1 ? 'bg-zinc-200 text-zinc-600' :
                                    idx === 2 ? 'bg-amber-100 text-amber-700' : 'text-zinc-400 bg-transparent'
                                }`}>
                                {idx + 1}
                            </div>
                            <span className="font-semibold text-zinc-700 text-sm group-hover:text-zinc-900">{user.name}</span>
                        </div>
                        <span className="font-bold text-zinc-900 text-sm bg-zinc-100 px-2 py-0.5 rounded text-xs">{user.score.toLocaleString()}</span>
                    </div>
                ))
            )}
        </CardContent>
    </Card>
);

export default Leaderboard;
