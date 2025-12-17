import React, { useEffect } from 'react';
import Leaderboard from './Leaderboard';
import CreateRoomPanel from './CreateRoomPanel';
import RoomListPage from './RoomListPage';
import { Card, CardContent } from "./ui/card";
import { Button } from "./ui/button";

const HomePage = ({ username, score, onLogout, onCreateRoom, onJoinRoom, rooms, leaderboard, onRequestLeaderboard }) => {
    // Initial load only
    useEffect(() => {
        if (onRequestLeaderboard) {
            onRequestLeaderboard();
        }
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, []);

    return (
        <div className="bg-[#f4f4f5] p-8 w-full max-w-7xl h-[85vh] flex flex-col gap-8 shadow-inner rounded-3xl border border-white/50">
            {/* Header */}
            <Card className="flex justify-between items-center p-5 border-transparent shadow-sm bg-white/80 backdrop-blur-sm">
                <div className="flex items-center gap-4">
                    <div className="w-12 h-12 bg-zinc-900 text-white rounded-full flex items-center justify-center text-xl font-bold shadow-md">
                        {username ? username.charAt(0).toUpperCase() : '?'}
                    </div>
                    <div>
                        <h1 className="text-xl font-bold text-zinc-900">{username}</h1>
                        <p className="text-sm font-semibold text-zinc-500 flex items-center gap-1">
                            <span className="text-amber-500">⭐️</span> {score} points
                        </p>
                    </div>
                </div>
                <Button
                    variant="ghost"
                    className="text-red-600 hover:text-red-700 hover:bg-red-50"
                    onClick={onLogout}
                >
                    Đăng xuất
                </Button>
            </Card>

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
