import React, { useEffect } from 'react';
import Leaderboard from './Leaderboard';
import CreateRoomPanel from './CreateRoomPanel';
import RoomListPage from './RoomListPage';

const HomePage = ({ username, score, onLogout, onCreateRoom, onJoinRoom, rooms, leaderboard, onRequestLeaderboard }) => {
    // Initial load only
    useEffect(() => {
        if (onRequestLeaderboard) {
            onRequestLeaderboard();
        }
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, []);

    return (
        <div className="bg-gray-100 p-8 w-full max-w-6xl h-[80vh] flex flex-col gap-6">
            {/* Header */}
            <div className="flex justify-between items-center bg-white p-4 rounded-xl shadow-sm border border-gray-200">
                <div className="flex items-center gap-4">
                    <div className="w-12 h-12 bg-blue-600 text-white rounded-full flex items-center justify-center text-xl font-bold shadow-md">
                        {username ? username.charAt(0).toUpperCase() : '?'}
                    </div>
                    <div>
                        <h1 className="text-xl font-bold text-gray-900">{username}</h1>
                        <p className="text-sm font-semibold text-yellow-600 flex items-center gap-1">
                            <span className="text-yellow-500">⭐️</span> {score} điểm
                        </p>
                    </div>
                </div>
                <button
                    className="text-red-500 hover:text-red-700 font-medium text-sm transition px-4 py-2 hover:bg-red-50 rounded-lg"
                    onClick={onLogout}
                >
                    Đăng xuất
                </button>
            </div>

            {/* Content Grid */}
            <div className="flex gap-6 flex-1 overflow-hidden">
                {/* Left Column: Leaderboard & Create */}
                <div className="w-1/3 flex flex-col gap-6">
                    <CreateRoomPanel onCreate={onCreateRoom} />
                    <div className="flex-1 overflow-hidden flex flex-col">
                        <Leaderboard data={leaderboard} onRefresh={onRequestLeaderboard} />
                    </div>
                </div>

                {/* Right Column: Room List */}
                <div className="flex-1 h-full">
                    <RoomListPage rooms={rooms} onJoin={onJoinRoom} />
                </div>
            </div>
        </div>
    );
};

export default HomePage;
