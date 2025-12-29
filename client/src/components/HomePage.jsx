import React, { useState, useEffect } from "react";
import Leaderboard from "./Leaderboard";
import CreateRoomPanel from "./CreateRoomPanel";
import RoomListPage from "./RoomListPage";
import ReplayModal from "./ReplayModal";
import { OPS } from "../lib/ops";

const HomePage = ({
  username,
  userId,
  score,
  socket,
  onLogout,
  onCreateRoom,
  onJoinRoom,
  rooms,
  leaderboard,
  onRequestLeaderboard,
}) => {
  const [activeTab, setActiveTab] = useState('rooms'); // 'rooms' or 'history'
  const [games, setGames] = useState([]);
  const [replayGame, setReplayGame] = useState(null);

  useEffect(() => {
    if (onRequestLeaderboard) {
      onRequestLeaderboard();
    }

    const intervalId = setInterval(() => {
      if (onRequestLeaderboard) {
        onRequestLeaderboard();
      }
    }, 5000);

    return () => clearInterval(intervalId);
  }, []);

  // Fetch history when switching to history tab
  useEffect(() => {
    if (activeTab === 'history' && userId) {
      const packet = new Uint8Array(1);
      packet[0] = OPS.GET_GAME_HISTORY;
      socket.emit('client_to_server', packet);

      const handleServerMessage = (data) => {
        const view = new Uint8Array(data);
        const opcode = view[0];

        if (opcode === OPS.GAME_HISTORY_RESPONSE) {
          const textDecoder = new TextDecoder();
          const payload = textDecoder.decode(view.slice(1));

          if (!payload) {
            setGames([]);
            return;
          }

          const gameEntries = payload.split(';').filter(e => e);
          const parsedGames = gameEntries.map(entry => {
            const [gameId, roomName, winnerId, timestamp, gameMode, score, rank, logData] = entry.split('|');
            return {
              gameId: parseInt(gameId),
              roomName,
              winnerId: parseInt(winnerId),
              timestamp,
              gameMode: parseInt(gameMode),
              score: parseInt(score),
              rank: parseInt(rank),
              logData
            };
          });

          setGames(parsedGames);
        }
      };

      socket.on('server_to_client', handleServerMessage);
      return () => socket.off('server_to_client', handleServerMessage);
    }
  }, [activeTab, userId]);

  return (
    <div className="bg-gray-100 p-8 w-full max-w-7xl flex flex-col gap-8 shadow-inner rounded-3xl border border-white/50 my-8">
      {/* Header */}
      <div className="flex justify-between items-center p-5 border-2 border-gray-200 shadow-sm bg-white/80 backdrop-blur-sm sticky top-4 z-10 rounded-xl">
        <div className="flex items-center gap-4">
          <div className="w-12 h-12 bg-black text-white rounded-full flex items-center justify-center text-xl font-bold shadow-md">
            {username ? username.charAt(0).toUpperCase() : "?"}
          </div>
          <div>
            <h1 className="text-xl font-bold text-black">{username}</h1>
            <p className="text-sm font-semibold text-gray-600 flex items-center gap-1">
              <span className="text-amber-500">⭐️</span> {score} points
            </p>
          </div>
        </div>
        <div className="flex gap-2">
          <button
            onClick={onLogout}
            className="px-4 py-2 text-red-600 hover:text-red-700 hover:bg-red-50 rounded-lg transition"
          >
            Đăng xuất
          </button>
        </div>
      </div>

      {/* Content Grid */}
      <div className="flex flex-col md:flex-row gap-6">
        {/* Left Column: Leaderboard & Create */}
        <div className="w-full md:w-1/3 flex flex-col gap-6">
          <CreateRoomPanel onCreate={onCreateRoom} />
          <div className="flex flex-col">
            <Leaderboard data={leaderboard} onRefresh={onRequestLeaderboard} />
          </div>
        </div>

        {/* Right Column: Tabs (Rooms / History) */}
        <div className="flex-1">
          {/* Tab Headers */}
          <div className="flex gap-2 mb-4">
            <button
              onClick={() => setActiveTab('rooms')}
              className={`flex-1 py-3 px-4 font-semibold rounded-t-xl transition ${activeTab === 'rooms'
                ? 'bg-white text-black border-2 border-b-0 border-gray-200'
                : 'bg-gray-200 text-gray-600 hover:bg-gray-300'
                }`}
            >
              🎮 Phòng Chơi
            </button>
            <button
              onClick={() => setActiveTab('history')}
              className={`flex-1 py-3 px-4 font-semibold rounded-t-xl transition ${activeTab === 'history'
                ? 'bg-white text-black border-2 border-b-0 border-gray-200'
                : 'bg-gray-200 text-gray-600 hover:bg-gray-300'
                }`}
            >
              📜 Lịch Sử
            </button>
          </div>

          {/* Tab Content */}
          {activeTab === 'rooms' ? (
            <RoomListPage rooms={rooms} onJoin={onJoinRoom} />
          ) : (
            <div className="bg-white border-2 border-gray-200 rounded-xl rounded-tl-none shadow-lg p-6">
              <h3 className="text-xl font-bold mb-4 text-black">📜 Lịch Sử Trận Đấu</h3>
              {games.length === 0 ? (
                <div className="text-center py-12 text-gray-400">
                  <p className="text-lg">Chưa có trận đấu nào</p>
                  <p className="text-sm mt-2">Chơi game để xây dựng lịch sử!</p>
                </div>
              ) : (
                <div className="space-y-3">
                  {games.map((game) => (
                    <div
                      key={game.gameId}
                      onClick={() => setReplayGame(game)}
                      className="p-4 border-2 border-gray-200 rounded-lg hover:border-indigo-400 hover:shadow-md transition cursor-pointer group"
                    >
                      <div className="flex justify-between items-center">
                        <div>
                          <h4 className="font-bold text-black group-hover:text-indigo-600 transition">
                            {game.roomName}
                          </h4>
                          <p className="text-sm text-gray-600">
                            {game.gameMode === 1 ? '⚔️ Loại trừ' : '⚡ Tính điểm'} •
                            Hạng {game.rank} • {game.score} điểm
                          </p>
                          <p className="text-xs text-gray-500 mt-1">{game.timestamp}</p>
                        </div>
                        <div className="flex items-center gap-2">
                          {game.winnerId === userId && (
                            <span className="text-yellow-500 text-2xl">🏆</span>
                          )}
                          <button className="px-3 py-1 bg-indigo-500 text-white rounded-lg text-sm hover:bg-indigo-600 transition">
                            ▶️ Xem
                          </button>
                        </div>
                      </div>
                    </div>
                  ))}
                </div>
              )}
            </div>
          )}
        </div>
      </div>

      {/* Replay Modal */}
      {replayGame && (
        <ReplayModal
          game={replayGame}
          userId={userId}
          socket={socket}
          onClose={() => setReplayGame(null)}
        />
      )}
    </div>
  );
};

export default HomePage;
