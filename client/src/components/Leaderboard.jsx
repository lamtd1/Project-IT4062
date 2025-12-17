import React from 'react';

const Leaderboard = ({ data }) => (
    <div className="bg-white p-4 rounded-xl shadow-sm border border-gray-100">
        <h3 className="font-bold text-gray-800 mb-3 text-lg flex items-center gap-2">
            <span className="text-yellow-500">🏆</span> Bảng Xếp Hạng
        </h3>
        <div className="space-y-2">
            {data.length === 0 ? (
                <p className="text-gray-400 text-sm text-center">Chưa có dữ liệu</p>
            ) : (
                data.map((user, idx) => (
                    <div key={idx} className="flex justify-between items-center p-2 bg-gray-50 rounded-lg">
                        <div className="flex items-center gap-3">
                            <span className={`font-bold w-6 text-center ${idx < 3 ? 'text-yellow-600' : 'text-gray-500'}`}>
                                #{idx + 1}
                            </span>
                            <span className="font-medium text-gray-700">{user.name}</span>
                        </div>
                        <span className="font-bold text-blue-600">{user.score}</span>
                    </div>
                ))
            )}
        </div>
    </div>
);

export default Leaderboard;
