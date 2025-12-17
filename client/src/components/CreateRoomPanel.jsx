import React, { useState } from 'react';

const CreateRoomPanel = ({ onCreate }) => {
    const [name, setName] = useState("");
    return (
        <div className="bg-white p-4 rounded-xl shadow-sm border border-gray-100 mt-4">
            <h3 className="font-bold text-gray-800 mb-3">Tạo phòng mới</h3>
            <div className="flex gap-2">
                <input
                    type="text"
                    placeholder="Tên phòng..."
                    className="flex-1 px-3 py-2 border rounded-lg text-sm outline-none focus:border-blue-500"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                />
                <button
                    onClick={() => name && onCreate(name)}
                    className="bg-purple-600 hover:bg-purple-700 text-white px-4 py-2 rounded-lg font-medium text-sm transition"
                >
                    + Tạo
                </button>
            </div>
        </div>
    );
}

export default CreateRoomPanel;
