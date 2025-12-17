import React, { useState } from 'react';
import { Card, CardContent, CardHeader, CardTitle, CardDescription, CardFooter } from "./ui/card";
import { Input } from "./ui/input";
import { Button } from "./ui/button";
import { cn } from "../lib/utils";

const CreateRoomPanel = ({ onCreate }) => {
    const [name, setName] = useState("");
    const [mode, setMode] = useState("1"); // Default Elimination

    return (
        <Card className="w-full max-w-md mx-auto mt-0 shadow-lg border-zinc-200 bg-white">
            <CardHeader>
                <div className="flex items-center justify-between">
                    <CardTitle className="text-xl flex items-center gap-2 text-zinc-900">
                        <span className="text-2xl">🎮</span> Tạo phòng mới
                    </CardTitle>
                    <span className="px-2 py-0.5 rounded text-[10px] uppercase font-bold tracking-wider bg-zinc-100 border border-zinc-200 text-zinc-500">
                        Beta
                    </span>
                </div>
                <CardDescription className="text-zinc-500">
                    Chọn chế độ và đặt tên để bắt đầu.
                </CardDescription>
            </CardHeader>
            <CardContent className="space-y-6">
                {/* Room Name */}
                <div className="space-y-2">
                    <label className="text-sm font-medium text-zinc-700">Tên phòng</label>
                    <Input
                        placeholder="Đặt tên phòng..."
                        value={name}
                        onChange={(e) => setName(e.target.value)}
                        className="bg-white border-zinc-300 text-zinc-900 placeholder:text-zinc-400 focus-visible:ring-indigo-500"
                    />
                </div>

                {/* Game Mode */}
                <div className="space-y-2">
                    <label className="text-sm font-medium text-zinc-700">Chế độ chơi</label>
                    <div className="grid grid-cols-3 gap-3">
                        {[
                            { id: "0", label: "Luyện tập", icon: "👤" },
                            { id: "1", label: "Loại trừ", icon: "⚔️" },
                            { id: "2", label: "Tính điểm", icon: "⚡" },
                        ].map((m) => (
                            <div
                                key={m.id}
                                onClick={() => setMode(m.id)}
                                className={cn(
                                    "cursor-pointer rounded-lg border p-3 flex flex-col items-center justify-center gap-2 transition-all duration-200",
                                    mode === m.id
                                        ? "bg-indigo-50 border-indigo-200 text-indigo-700 shadow-sm ring-1 ring-indigo-200"
                                        : "bg-white border-zinc-200 text-zinc-500 hover:bg-zinc-50 hover:border-zinc-300"
                                )}
                            >
                                <span className="text-lg">{m.icon}</span>
                                <span className="text-xs font-bold">{m.label}</span>
                            </div>
                        ))}
                    </div>
                </div>
            </CardContent>
            <CardFooter>
                <Button
                    className="w-full bg-zinc-900 text-white hover:bg-zinc-800 font-bold py-6 text-md shadow-md transition-all active:scale-[0.98]"
                    onClick={() => name && onCreate(`${name}:${mode}`)}
                    disabled={!name}
                >
                    Tạo phòng ngay
                </Button>
            </CardFooter>
        </Card>
    );
}

export default CreateRoomPanel;
