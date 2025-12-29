import React, { useState } from 'react';

const CreateRoomPanel = ({ onCreate }) => {
    const [name, setName] = useState("");
    const [mode, setMode] = useState("1"); // Default Elimination

    return (
        <div className="w-full max-w-md mx-auto mt-0 shadow-lg border-2 border-gray-200 bg-white rounded-xl">
            <div className="p-6">
                <div className="flex items-center justify-between mb-2">
                    <h3 className="text-xl font-semibold text-black">
                        <span className="text-2xl">🎮 Tạo phòng mới</span>
                    </h3>
                </div>
                <p className="text-gray-500 text-sm">Chọn chế độ và đặt tên để bắt đầu.</p>
            </div>
            <div className="space-y-6 px-6 pb-6">
                {/* Room Name */}
                <div className="space-y-2">
                    <label className="text-sm font-medium text-gray-700">Tên phòng</label>
                    <input
                        type="text"
                        placeholder="Đặt tên phòng..."
                        value={name}
                        onChange={(e) => setName(e.target.value)}
                        className="w-full px-4 py-2 border-2 border-gray-300 rounded-lg bg-white text-black placeholder:text-gray-400 focus:outline-none focus:border-indigo-500 transition"
                    />
                </div>

                {/* Game Mode */}
                <div className="space-y-2">
                    <label className="text-sm font-medium text-gray-700">Chế độ chơi</label>
                    <div className="grid grid-cols-3 gap-3">
                        {[
                            { id: "0", label: "Luyện tập", icon: "👤" },
                            { id: "1", label: "Loại trừ", icon: "⚔️" },
                            { id: "2", label: "Tính điểm", icon: "⚡" },
                        ].map((m) => (
                            <div
                                key={m.id}
                                onClick={() => setMode(m.id)}
                                className={`cursor-pointer rounded-lg border-2 p-3 flex flex-col items-center justify-center gap-2 transition-all duration-200 ${mode === m.id
                                        ? "bg-indigo-50 border-indigo-500 text-indigo-700 shadow-sm ring-2 ring-indigo-200"
                                        : "bg-white border-gray-300 text-gray-600 hover:bg-gray-50 hover:border-gray-400"
                                    }`}
                            >
                                <span className="text-lg">{m.icon}</span>
                                <span className="text-xs font-bold">{m.label}</span>
                            </div>
                        ))}
                    </div>
                </div>
            </div>
            <div className="p-6 pt-0">
                <button
                    onClick={() => name && onCreate(`${name}:${mode}`)}
                    disabled={!name}
                    className="w-full bg-black text-white hover:bg-gray-800 font-bold py-4 text-md rounded-lg shadow-md transition-all active:scale-[0.98] disabled:opacity-50 disabled:cursor-not-allowed disabled:hover:bg-black"
                >
                    🚀 Tạo phòng ngay
                </button>
            </div>
        </div>
    );
}

export default CreateRoomPanel;
