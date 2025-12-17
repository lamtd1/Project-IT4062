import React, { useState } from 'react';
import { Card, CardContent, CardHeader, CardTitle, CardDescription, CardFooter } from "./ui/card";
import { Button } from "./ui/button";
import { Badge } from "./ui/badge";

const RoomPage = ({ roomInfo, members, isHost, onLeave, onStart, idleUsers = [], onGetIdleUsers, onSendInvite }) => {
    const [showInvite, setShowInvite] = useState(false);

    const handleOpenInvite = () => {
        if (onGetIdleUsers) onGetIdleUsers();
        setShowInvite(true);
    };

    const handleInviteClick = (user) => {
        if (onSendInvite) onSendInvite(user);
        // Maybe show styling feedback?
    };

    return (
        <React.Fragment>
            {/* Invitation Modal */}
            {showInvite && (
                <div className="fixed inset-0 bg-black/50 backdrop-blur-sm z-50 flex items-center justify-center animate-in fade-in">
                    <Card className="w-full max-w-md bg-white shadow-xl border-zinc-200">
                        <CardHeader className="flex flex-row items-center justify-between">
                            <CardTitle className="text-xl">Mời bạn bè</CardTitle>
                            <Button variant="ghost" size="sm" onClick={() => setShowInvite(false)}>✕</Button>
                        </CardHeader>
                        <CardContent className="space-y-4">
                            <div className="flex justify-between items-center bg-zinc-50 p-2 rounded text-xs text-zinc-500">
                                <span>Người chơi đang rảnh: {idleUsers.length}</span>
                                <Button variant="link" size="sm" onClick={onGetIdleUsers} className="h-auto p-0">Làm mới</Button>
                            </div>
                            <div className="max-h-[300px] overflow-y-auto space-y-2">
                                {idleUsers.length === 0 ? (
                                    <div className="text-center py-8 text-zinc-400">Không có ai đang rảnh :(</div>
                                ) : (
                                    idleUsers.map((u, idx) => (
                                        <div key={idx} className="flex justify-between items-center p-3 rounded-lg border border-zinc-100 hover:bg-zinc-50 transition-colors">
                                            <div className="flex items-center gap-3">
                                                <div className="w-8 h-8 rounded-full bg-indigo-100 text-indigo-600 flex items-center justify-center text-xs font-bold font-mono">
                                                    {u.charAt(0).toUpperCase()}
                                                </div>
                                                <span className="font-semibold text-zinc-700">{u}</span>
                                            </div>
                                            <Button size="sm" variant="outline" onClick={() => handleInviteClick(u)} className="h-8 border-indigo-200 text-indigo-600 hover:bg-indigo-50">
                                                Mời
                                            </Button>
                                        </div>
                                    ))
                                )}
                            </div>
                        </CardContent>
                    </Card>
                </div>
            )}

            <Card className="max-w-5xl w-full h-[650px] flex shadow-2xl border-zinc-200 bg-white">
                <div className="flex-1 p-8 flex flex-col border-r border-zinc-100">
                    <div className="mb-8 flex justify-between items-start">
                        <div>
                            <h2 className="text-3xl font-bold text-zinc-900 tracking-tight mb-2">{roomInfo.name}</h2>
                            <div className="flex items-center gap-2">
                                <span className="px-2 py-1 rounded-md bg-zinc-100 text-zinc-600 font-mono text-xs border border-zinc-200">
                                    Room ID: {roomInfo.id}
                                </span>
                                <Badge variant={isHost ? "default" : "secondary"} className={isHost ? "bg-indigo-600" : ""}>
                                    {isHost ? "Host" : "Member"}
                                </Badge>
                            </div>
                        </div>
                    </div>

                    <Card className="flex-1 mb-6 bg-zinc-50 border-zinc-200 shadow-inner flex items-center justify-center p-6 relative overflow-hidden">
                        <div className="absolute inset-0 opacity-5 pattern-dots pattern-zinc-500 pattern-bg-transparent pattern-size-4" />

                        {isHost ? (
                            <div className="text-center w-full max-w-sm relative z-10">
                                <Button
                                    onClick={onStart}
                                    disabled={members.length < 1}
                                    size="lg"
                                    className={`w-full py-8 text-xl font-bold transition-all ${members.length < 1 ? 'opacity-50' : 'hover:scale-105 shadow-xl bg-indigo-600 hover:bg-indigo-700'
                                        }`}
                                >
                                    {members.length < 1 ? 'Chờ người chơi...' : 'BẮT ĐẦU GAME 🚀'}
                                </Button>
                                <p className="mt-4 text-zinc-500 text-sm font-medium">
                                    {members.length < 1 ? "Cần ít nhất 1 người chơi để bắt đầu" : "Đã sẵn sàng!"}
                                </p>
                            </div>
                        ) : (
                            <div className="text-center space-y-4 relative z-10">
                                <div className="w-16 h-16 border-4 border-zinc-200 border-t-indigo-600 rounded-full animate-spin mx-auto"></div>
                                <p className="text-zinc-600 font-medium">Đang chờ chủ phòng bắt đầu...</p>
                            </div>
                        )}
                    </Card>

                    <Button variant="outline" onClick={onLeave} className="self-start text-red-600 hover:text-red-700 hover:bg-red-50 border-red-200">
                        ← Rời phòng
                    </Button>
                </div>

                {/* Sidebar */}
                <div className="w-80 bg-zinc-50/50 p-6 flex flex-col border-l border-zinc-100">
                    <div className="flex justify-between items-center mb-6">
                        <h3 className="font-semibold text-zinc-900 flex items-center gap-2">
                            Người chơi
                            <Badge variant="secondary" className="bg-white border-zinc-200">{members.length}/4</Badge>
                        </h3>
                        {/* Only Host can invite? Or everyone? Protocol allows everyone but UI usually Host only. Let's allow Host only for control. */}
                        {isHost && (
                            <Button size="sm" variant="ghost" onClick={handleOpenInvite} className="h-8 px-2 text-indigo-600 hover:text-indigo-700 hover:bg-indigo-50">
                                + Mời
                            </Button>
                        )}
                    </div>

                    <div className="space-y-3 flex-1 overflow-y-auto">
                        {members.map((mem, idx) => (
                            <Card key={idx} className="p-3 flex items-center gap-3 border-zinc-200 shadow-sm transition-all hover:shadow-md bg-white">
                                <div className={`w-10 h-10 rounded-full flex items-center justify-center text-white font-bold text-sm shadow-sm ${mem.isHost ? 'bg-indigo-600' : 'bg-zinc-400'
                                    }`}>
                                    {mem.username.charAt(0).toUpperCase()}
                                </div>
                                <div className="flex-1 truncate">
                                    <div className="flex items-center gap-1.5">
                                        <span className="font-semibold text-sm text-zinc-900 truncate">{mem.username}</span>
                                        {mem.isHost && <span className="text-[10px] bg-indigo-50 text-indigo-700 px-1.5 py-0.5 rounded border border-indigo-100 font-bold">HOST</span>}
                                    </div>
                                    <div className="text-xs text-zinc-500">Điểm: {mem.score}</div>
                                </div>
                            </Card>
                        ))}
                        {/* Empty slots placeholders */}
                        {Array.from({ length: Math.max(0, 4 - members.length) }).map((_, i) => (
                            <div key={`empty-${i}`} className="p-3 border-2 border-dashed border-zinc-200 rounded-lg flex items-center gap-3 opacity-50">
                                <div className="w-10 h-10 rounded-full bg-zinc-100"></div>
                                <div className="h-4 bg-zinc-100 w-20 rounded"></div>
                            </div>
                        ))}
                    </div>
                </div>
            </Card>
        </React.Fragment>
    );
};

export default RoomPage;
