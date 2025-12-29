import React from 'react';

const Leaderboard = ({ data, onRefresh }) => (
    <div className="shadow-lg border-2 border-gray-200 bg-white rounded-xl overflow-hidden flex flex-col h-full">
        <div className="py-4 pb-3 border-b border-gray-100 bg-gray-50 px-6">
            <div className="flex items-center justify-between">
                <h3 className="text-lg font-bold flex items-center gap-2 text-black">
                    <span className="text-xl">🏆</span> Bảng Xếp Hạng
                </h3>
            </div>
        </div>
        <div className="space-y-1 p-2 flex-1 overflow-y-auto">
            {data.length === 0 ? (
                <div className="text-center py-10 text-gray-400 text-sm italic">Chưa có dữ liệu xếp hạng</div>
            ) : (
                data.map((user, idx) => (
                    <div key={idx} className="flex justify-between items-center p-2.5 rounded-md hover:bg-gray-50 transition-colors cursor-default group">
                        <div className="flex items-center gap-3">
                            <div className={`flex items-center justify-center w-6 h-6 rounded-full text-xs font-bold ${idx === 0 ? 'bg-yellow-100 text-yellow-700' :
                                    idx === 1 ? 'bg-gray-200 text-gray-600' :
                                        idx === 2 ? 'bg-amber-100 text-amber-700' : 'text-gray-400 bg-transparent'
                                }`}>
                                {idx + 1}
                            </div>
                            <span className="font-semibold text-gray-700 text-sm group-hover:text-black">{user.name}</span>
                        </div>
                        <span className="font-bold text-black text-sm bg-gray-100 px-2 py-0.5 rounded text-xs">{user.score.toLocaleString()}</span>
                    </div>
                ))
            )}
        </div>
    </div>
);

export default Leaderboard;
