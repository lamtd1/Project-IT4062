import { useState } from 'react';

export default function Lobby({ username, onJoinGame, onLogout }) {
    const [activeTab, setActiveTab] = useState('rooms');

    const mockRooms = [
        { id: 1, name: 'Room 1', players: 2, status: 'Waiting' },
        { id: 2, name: 'Room 2', players: 4, status: 'Full' },
        { id: 3, name: 'Room 3', players: 1, status: 'Playing' },
    ];

    const mockUsers = [
        { id: 1, name: 'Player 1', status: 'Online' },
        { id: 2, name: 'Player 2', status: 'In Game' },
        { id: 3, name: 'Player 3', status: 'Online' },
    ];

    return (
        <div className="p-8 max-w-6xl mx-auto h-screen flex flex-col">
            <header className="flex justify-between items-center mb-8 bg-gray-800/50 p-4 rounded-xl border border-gray-700">
                <div className="flex items-center gap-4">
                    <div className="w-12 h-12 bg-gradient-to-br from-blue-500 to-purple-600 rounded-full flex items-center justify-center text-xl font-bold">
                        {username[0]?.toUpperCase()}
                    </div>
                    <div>
                        <h2 className="text-xl font-bold text-white">{username}</h2>
                        <span className="text-sm text-green-400">● Online</span>
                    </div>
                </div>
                <button
                    onClick={onLogout}
                    className="px-6 py-2 bg-red-600/80 hover:bg-red-500 rounded-lg transition-colors">
                    Logout
                </button>
            </header>

            <div className="flex-1 grid grid-cols-12 gap-6">
                {/* Main Content Area */}
                <div className="col-span-8 bg-gray-800/30 rounded-xl border border-gray-700 p-6 flex flex-col">
                    <div className="flex gap-4 mb-6 border-b border-gray-700 pb-4">
                        <button
                            onClick={() => setActiveTab('rooms')}
                            className={`px-6 py-2 rounded-lg font-medium transition-all ${activeTab === 'rooms'
                                ? 'bg-blue-600 text-white shadow-lg shadow-blue-500/20'
                                : 'text-gray-400 hover:text-white hover:bg-gray-700'
                                }`}
                        >
                            Game Rooms
                        </button>
                        <button
                            onClick={() => setActiveTab('users')}
                            className={`px-6 py-2 rounded-lg font-medium transition-all ${activeTab === 'users'
                                ? 'bg-blue-600 text-white shadow-lg shadow-blue-500/20'
                                : 'text-gray-400 hover:text-white hover:bg-gray-700'
                                }`}
                        >
                            Online Users
                        </button>
                    </div>

                    <div className="flex-1 overflow-y-auto space-y-3">
                        {activeTab === 'rooms' ? (
                            mockRooms.map((room) => (
                                <div
                                    key={room.id}
                                    className="flex items-center justify-between p-4 bg-gray-700/30 rounded-lg hover:bg-gray-700/50 transition-colors border border-gray-700/50"
                                >
                                    <div>
                                        <h3 className="font-bold text-lg">{room.name}</h3>
                                        <p className="text-sm text-gray-400">{room.players}/4 Players</p>
                                    </div>
                                    <div className="flex items-center gap-4">
                                        <span className={`px-3 py-1 rounded-full text-xs font-medium ${room.status === 'Waiting' ? 'bg-green-500/20 text-green-400' :
                                            room.status === 'Full' ? 'bg-red-500/20 text-red-400' :
                                                'bg-yellow-500/20 text-yellow-400'
                                            }`}>
                                            {room.status}
                                        </span>
                                        <button
                                            onClick={onJoinGame}
                                            disabled={room.status !== 'Waiting'}
                                            className="px-4 py-2 bg-blue-600 hover:bg-blue-500 disabled:opacity-50 disabled:cursor-not-allowed rounded-lg text-sm font-medium transition-colors"
                                        >
                                            Join
                                        </button>
                                    </div>
                                </div>
                            ))
                        ) : (
                            mockUsers.map((user) => (
                                <div
                                    key={user.id}
                                    className="flex items-center justify-between p-4 bg-gray-700/30 rounded-lg border border-gray-700/50"
                                >
                                    <div className="flex items-center gap-3">
                                        <div className="w-8 h-8 bg-gray-600 rounded-full flex items-center justify-center text-sm">
                                            {user.name[0]}
                                        </div>
                                        <span className="font-medium">{user.name}</span>
                                    </div>
                                    <span className="text-sm text-gray-400">{user.status}</span>
                                </div>
                            ))
                        )}
                    </div>
                </div>

                {/* Sidebar / Quick Actions */}
                <div className="col-span-4 space-y-6">
                    <div className="bg-gray-800/30 rounded-xl border border-gray-700 p-6">
                        <h3 className="font-bold text-lg mb-4">Quick Actions</h3>
                        <button
                            onClick={onJoinGame}
                            className="w-full py-3 bg-gradient-to-r from-green-600 to-green-500 hover:from-green-500 hover:to-green-400 rounded-lg font-bold shadow-lg shadow-green-500/20 transition-all mb-3"
                        >
                            Create Room
                        </button>
                        <button className="w-full py-3 bg-gray-700 hover:bg-gray-600 rounded-lg font-medium transition-colors">
                            Find Match
                        </button>
                    </div>

                    <div className="bg-gray-800/30 rounded-xl border border-gray-700 p-6">
                        <h3 className="font-bold text-lg mb-4">Leaderboard</h3>
                        <div className="space-y-3">
                            {[1, 2, 3].map((i) => (
                                <div key={i} className="flex items-center justify-between text-sm">
                                    <div className="flex items-center gap-2">
                                        <span className="text-yellow-500 font-bold">#{i}</span>
                                        <span>Player {i}</span>
                                    </div>
                                    <span className="text-gray-400">1,000,000</span>
                                </div>
                            ))}
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
}
