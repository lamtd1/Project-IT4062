import { useState, useEffect, useCallback } from 'react';
import { useNavigate } from 'react-router-dom';
import io from 'socket.io-client';
import { OPS } from '../lib/ops';

const socket = io('http://localhost:4000');

export const useGameLogic = () => {
    const navigate = useNavigate();

    // Initialize from localStorage if available
    const [username, setUsername] = useState(() => localStorage.getItem('username') || '');
    const [password, setPassword] = useState('');
    const [userId, setUserId] = useState(() => {
        const saved = localStorage.getItem('userId');
        return saved ? parseInt(saved) : null;
    });
    const [score, setScore] = useState(() => {
        const saved = localStorage.getItem('score');
        return saved ? parseInt(saved) : 0;
    });
    const [userRole, setUserRole] = useState(() => {
        const saved = localStorage.getItem('userRole');
        return saved ? parseInt(saved) : 1;
    });
    const [status, setStatus] = useState({ msg: '', type: '' });

    // Game State
    const [gameStatus, setGameStatus] = useState('LOBBY'); // LOBBY, PLAYING, FINISHED
    const [currentQuestion, setCurrentQuestion] = useState(null);
    const [timeLeft, setTimeLeft] = useState(0);
    const [gameResult, setGameResult] = useState("");
    const [renderKey, setRenderKey] = useState(0);
    const [players, setPlayers] = useState([]); // Live scores: [{username, score}, ...]
    const [wrongAnswer, setWrongAnswer] = useState(false); // Mode 2: Dim UI when wrong
    const [currentGameScore, setCurrentGameScore] = useState(0); // Track my current game score

    const [roomInfo, setRoomInfo] = useState({ id: null, name: '' });
    const [roomMembers, setRoomMembers] = useState([]);
    const [isHost, setIsHost] = useState(false);

    const [rooms, setRooms] = useState([]);
    const [leaderboard, setLeaderboard] = useState([]);

    // Invitation State
    const [idleUsers, setIdleUsers] = useState([]);
    const [incomingInvite, setIncomingInvite] = useState(null);

    // Admin State
    const [allUsers, setAllUsers] = useState([]); // For admin panel
    const [selectedUserDetail, setSelectedUserDetail] = useState(null);

    // --- SOCKET HANDLERS ---
    useEffect(() => {
        // Poll rooms and leaderboard periodically
        const interval = setInterval(() => {
            if (window.location.pathname === '/home') {
                const p1 = new Uint8Array(1); p1[0] = OPS.GET_ROOMS;
                socket.emit("client_to_server", p1);
            }
            else if (window.location.pathname === '/room') {
                if (roomInfo.id !== null && roomInfo.id !== undefined) {
                    const textData = `${roomInfo.id}`;
                    const encoder = new TextEncoder();
                    const stringBytes = encoder.encode(textData);
                    const packet = new Uint8Array(1 + stringBytes.length);
                    packet[0] = OPS.GET_ROOM_DETAIL;
                    packet.set(stringBytes, 1);
                    socket.emit("client_to_server", packet);
                }
            }
        }, 5000);

        socket.on('server_to_client', (data) => {
            const view = new Uint8Array(data);
            const opcode = view[0];
            const textDecoder = new TextDecoder();

            if (opcode === OPS.LOGIN_SUCCESS) {
                const payload = textDecoder.decode(view.slice(1));
                const [idStr, scoreStr, roleStr] = payload.split(':');
                const role = parseInt(roleStr || '1');
                const uid = parseInt(idStr);
                const sc = parseInt(scoreStr || '0');

                // Save to state
                setUserId(uid);
                setScore(sc);
                setUserRole(role);

                // Persist to localStorage
                localStorage.setItem('userId', uid.toString());
                localStorage.setItem('username', username);
                localStorage.setItem('score', sc.toString());
                localStorage.setItem('userRole', role.toString());

                setStatus({ msg: "Đăng nhập thành công!", type: 'success' });
                setTimeout(() => navigate(role === 0 ? '/admin' : '/home'), 500);
            }
            else if (opcode === OPS.LOGIN_FAILED) {
                setStatus({ msg: "Sai tài khoản hoặc mật khẩu!", type: 'error' });
            }
            else if (opcode === OPS.REGISTER_SUCCESS) {
                setStatus({ msg: "Đăng ký thành công!", type: 'success' });
                setTimeout(() => { setStatus({ msg: '', type: '' }); navigate('/'); }, 1500);
            }
            else if (opcode === OPS.REGISTER_FAILED) setStatus({ msg: "Tài khoản đã tồn tại.", type: 'error' });
            else if (opcode === OPS.ALREADY_LOGIN) setStatus({ msg: "Bạn đã đăng nhập rồi!", type: 'error' });
            else if (opcode === OPS.SERVER_FULL) setStatus({ msg: "Server đã đầy!", type: 'error' });

            // --- ROOM RESPONSE ---
            else if (opcode === OPS.ROOM_CREATE) {
                const success = view[1];
                if (success) {
                    const rId = view[2];
                    setRoomInfo(prev => ({ ...prev, id: rId }));
                    setIsHost(true);
                    setStatus({ msg: `Tạo phòng ${rId} thành công!`, type: 'success' });
                    navigate('/room');
                } else setStatus({ msg: "Tạo phòng thất bại!", type: 'error' });
            }
            else if (opcode === OPS.ROOM_JOIN) {
                const success = view[1];
                if (success) {
                    setIsHost(false);
                    navigate('/room');
                } else setStatus({ msg: "Vào phòng thất bại!", type: 'error' });
            }
            else if (opcode === OPS.LEAVE_ROOM) {
                alert("Phòng đã bị hủy do chủ phòng thoát!");
                navigate('/home');
                setRoomMembers([]);
                setRoomInfo({});
                setGameStatus('LOBBY');
            }

            // --- GAME RESPONSES ---
            else if (opcode === 0x21) { // MSG_QUESTION
                const payload = textDecoder.decode(view.slice(1));
                const parts = payload.split('|');
                if (parts.length >= 7) {
                    const [lvl, content, a, b, c, d, dur] = parts;
                    const newQuestion = {
                        id: parseInt(lvl),
                        content: content,
                        answers: [a, b, c, d]
                    };
                    console.log(`[GAME] Question ${lvl}: ${content}`);
                    setCurrentQuestion(newQuestion);
                    setTimeLeft(parseInt(dur));
                    setGameStatus('PLAYING');
                    setWrongAnswer(false); // Reset dimming for new question
                    setRenderKey(prev => prev + 1);
                }
            }
            else if (opcode === 0x23) { // MSG_ANSWER_RESULT
                const payload = textDecoder.decode(view.slice(1));

                // Parse extended protocol: "message|eliminated:0/1|wrong_answer:0/1"
                const parts = payload.split('|');
                const message = parts[0];
                const eliminated = parts[1]?.includes('eliminated:1');
                const wrongAnswerFlag = parts[2]?.includes('wrong_answer:1');

                // Parse score from message "Tong: 200" to update immediately (fixes timing bug)
                const scoreMatch = message.match(/Tong:\s*(\d+)/);
                let finalScore = currentGameScore; // Default to current
                if (scoreMatch) {
                    finalScore = parseInt(scoreMatch[1]);
                    setCurrentGameScore(finalScore);
                    console.log('[SCORE] Updated from answer result:', finalScore);
                }

                // Handle elimination (Mode 1)
                if (eliminated) {
                    console.log('[ELIMINATED]', message, 'Score:', finalScore);

                    // Request updated user info from server (binary protocol)
                    if (socket?.connected) {
                        const refreshMsg = new Uint8Array(1);
                        refreshMsg[0] = OPS.REFRESH_USER_INFO;
                        socket.emit('client_to_server', refreshMsg);
                        console.log('[USER_INFO] Requested refresh from server');
                    }

                    // Reset game state and return to lobby
                    setGameStatus('FINISHED');
                    setGameResult(`BẠN ĐÃ BỊ LOẠI!\n\n${message}\n\nĐiểm game: ${finalScore}`);
                    setCurrentQuestion(null);
                    setWrongAnswer(false);
                    setCurrentGameScore(0); // Reset for next game
                    return; // Don't process further
                }

                // Set wrong answer flag for UI dimming (Mode 2)
                setWrongAnswer(wrongAnswerFlag || false);

                // Log result (no blocking alerts for smooth gameplay)
                if (wrongAnswerFlag) {
                    console.log('[WRONG]', message);
                } else {
                    console.log('[ANSWER]', message);
                }

                // Question will auto-update when server broadcasts next question
            }
            else if (opcode === OPS.SCORE_UPDATE) { // 0x25
                const payload = textDecoder.decode(view.slice(1));
                // Format: "username1:score1,username2:score2,..."
                if (payload) {
                    const playerList = payload.split(',').filter(x => x).map(item => {
                        const [name, sc] = item.split(':');
                        return { username: name, score: parseInt(sc || 0) };
                    });
                    setPlayers(playerList);

                    // Update my current game score
                    const me = playerList.find(p => p.username === username);
                    if (me) setCurrentGameScore(me.score);
                    console.log('[SCORE_UPDATE]', playerList);
                }
            }
            else if (opcode === 0x26) { // MSG_GAME_END
                const payload = textDecoder.decode(view.slice(1));
                setGameStatus('FINISHED');
                setGameResult(payload);

                // Request updated user info (covers Walk Away & Game Over)
                if (socket?.connected && userId) {
                    const refreshMsg = new Uint8Array(1);
                    refreshMsg[0] = OPS.REFRESH_USER_INFO;
                    socket.emit('client_to_server', refreshMsg);
                    console.log('[USER_INFO] Requested refresh on GAME_END');
                }
            }
            else if (opcode === 0x2B) { // MSG_WALK_AWAY
                const msg = textDecoder.decode(view.slice(1));
                alert(msg);
            } else if (opcode === 0x2D) { // MSG_HELP_RESULT
                const msg = textDecoder.decode(view.slice(1));
                alert(`Tư vấn trợ giúp:\n${msg}`);
            }
            else if (opcode === OPS.ROOM_LIST) {
                const listStr = textDecoder.decode(view.slice(1));
                if (!listStr) setRooms([]);
                else {
                    setRooms(listStr.split(',').map(item => {
                        const [id, name, count, rStatus] = item.split(':');
                        return { id, name, count, status: rStatus };
                    }));
                }
            }
            else if (opcode === OPS.LEADERBOARD_LIST) {
                const listStr = textDecoder.decode(view.slice(1));
                if (!listStr) setLeaderboard([]);
                else {
                    setLeaderboard(listStr.split(',').map(item => {
                        const [name, sc] = item.split(':');
                        return { name, score: sc };
                    }));
                }
            }
            else if (opcode === OPS.USER_INFO_UPDATE) {
                const payload = textDecoder.decode(view.slice(1));
                const newScore = parseInt(payload || '0');
                setScore(newScore);
                console.log('[USER_INFO] Updated total_score:', newScore);
            }

            else if (opcode === OPS.ROOM_DETAIL) {
                const listStr = textDecoder.decode(view.slice(1));
                if (!listStr) setRoomMembers([]);
                else {
                    const mems = listStr.split(',').map(item => {
                        const [isH, name, sc] = item.split(':');
                        return { isHost: isH === '1', username: name, score: sc };
                    });
                    setRoomMembers(mems);
                }
            }
            else if (opcode === OPS.IDLE_USERS_LIST) {
                const listStr = textDecoder.decode(view.slice(1));
                if (!listStr) setIdleUsers([]);
                else setIdleUsers(listStr.split(',').filter(x => x));
            }
            else if (opcode === OPS.INVITE_RECEIVED) {
                const payload = textDecoder.decode(view.slice(1));
                const [hostName, roomId] = payload.split(':');
                setIncomingInvite({ hostName, roomId });
                setTimeout(() => setIncomingInvite(null), 10000);
            }
            // --- ADMIN RESPONSES ---
            else if (opcode === OPS.ALL_USERS_RESULT) {
                const listStr = textDecoder.decode(view.slice(1));
                if (!listStr) setAllUsers([]);
                else {
                    const users = listStr.split(',').map(item => {
                        const [id, username, total_win, total_score, is_online] = item.split(':');
                        return {
                            id: parseInt(id),
                            username,
                            total_win: parseInt(total_win),
                            total_score: parseInt(total_score),
                            is_online: parseInt(is_online) === 1
                        };
                    });
                    // Sort: online first, then by username
                    users.sort((a, b) => {
                        if (a.is_online !== b.is_online) return b.is_online - a.is_online;
                        return a.username.localeCompare(b.username);
                    });
                    setAllUsers(users);
                }
            }
            else if (opcode === OPS.DELETE_USER_RESULT) {
                const success = view[1] === 49; // ASCII '1'
                if (success) {
                    alert('Xóa người dùng thành công!');
                    // Refresh user list
                    const packet = new Uint8Array(1);
                    packet[0] = OPS.GET_ALL_USERS;
                    socket.emit("client_to_server", packet);
                } else {
                    alert('Xóa người dùng thất bại!');
                }
            }
            else if (opcode === OPS.USER_DETAIL_RESULT) {
                const payload = textDecoder.decode(view.slice(1));
                if (payload) {
                    const [id, username, total_win, total_score] = payload.split(':');
                    setSelectedUserDetail({
                        id: parseInt(id),
                        username,
                        total_win: parseInt(total_win),
                        total_score: parseInt(total_score)
                    });
                }
            }
        });

        return () => { socket.off('server_to_client'); clearInterval(interval); };
    }, [navigate, roomInfo.id, username, userId, currentGameScore]);

    // Timer Countdown Effect
    useEffect(() => {
        if (gameStatus === 'PLAYING' && timeLeft > 0) {
            const timer = setInterval(() => {
                setTimeLeft(prev => prev - 1);
            }, 1000);
            return () => clearInterval(timer);
        }
    }, [gameStatus, timeLeft]);


    // --- ACTION HANDLERS ---
    const handleSubmit = (opcode) => {
        if (!username || !password) return setStatus({ msg: "Nhập đủ thông tin", type: 'error' });
        const text = `${username} ${password}`;
        const encoder = new TextEncoder();
        const bytes = encoder.encode(text);
        const packet = new Uint8Array(1 + bytes.length);
        packet[0] = opcode;
        packet.set(bytes, 1);
        socket.emit("client_to_server", packet);
    };

    const handleCreateRoom = (roomName) => {
        const encoder = new TextEncoder();
        const bytes = encoder.encode(roomName);
        const packet = new Uint8Array(1 + bytes.length);
        packet[0] = OPS.ROOM_CREATE;
        packet.set(bytes, 1);
        socket.emit("client_to_server", packet);
        setRoomInfo({ id: 0, name: roomName });
        setGameStatus('LOBBY');
    };

    const handleJoinRoom = (roomId) => {
        const packet = new Uint8Array(1 + roomId.length);
        packet[0] = OPS.ROOM_JOIN;
        const encoder = new TextEncoder();
        packet.set(encoder.encode(roomId), 1);
        socket.emit("client_to_server", packet);

        const r = rooms.find(rm => rm.id === roomId);
        setRoomInfo({ id: roomId, name: r ? r.name : `Phòng ${roomId}` });
        setGameStatus('LOBBY');
    };

    const handleLeaveRoom = () => {
        const packet = new Uint8Array(1);
        packet[0] = OPS.LEAVE_ROOM;
        socket.emit("client_to_server", packet);
        navigate('/home');
        setRoomMembers([]);
        setGameStatus('LOBBY');
    };

    const handleLogout = () => {
        const packet = new Uint8Array(1);
        packet[0] = OPS.LOGOUT;
        socket.emit("client_to_server", packet);

        // Clear localStorage
        localStorage.removeItem('userId');
        localStorage.removeItem('username');
        localStorage.removeItem('score');
        localStorage.removeItem('userRole');

        // Reset state
        setUserId(null);
        setUsername('');
        setScore(0);
        setUserRole(1); // Assuming 1 is the default user role
        setGameStatus('LOBBY');
        setRoomInfo({ id: null, name: '' });
        setRoomMembers([]);
        navigate('/');
    };

    const handleGetLeaderboard = () => {
        if (!socket) return;
        const packet = new Uint8Array(1);
        packet[0] = 0x45;
        socket.emit("client_to_server", packet);
    };

    const handleStartGame = () => {
        const packet = new Uint8Array(1);
        packet[0] = 0x20;
        socket.emit("client_to_server", packet);
        console.log("Sent GAME_START to server");
    };

    const handleAnswer = (ansChar) => {
        const encoder = new TextEncoder();
        const bytes = encoder.encode(ansChar);
        const packet = new Uint8Array(1 + bytes.length);
        packet[0] = 0x22;
        packet.set(bytes, 1);
        socket.emit("client_to_server", packet);
    };

    const handleGetIdleUsers = () => {
        const packet = new Uint8Array(1);
        packet[0] = OPS.GET_IDLE_USERS;
        socket.emit("client_to_server", packet);
    };

    const handleSendInvite = (targetUser) => {
        const encoder = new TextEncoder();
        const bytes = encoder.encode(targetUser);
        const packet = new Uint8Array(1 + bytes.length);
        packet[0] = OPS.INVITE_FRIEND;
        packet.set(bytes, 1);
        socket.emit("client_to_server", packet);
    };

    const handleAcceptInvite = () => {
        if (incomingInvite) {
            handleJoinRoom(incomingInvite.roomId);
            setIncomingInvite(null);
        }
    };

    const handleReplay = () => {
        console.log('[REPLAY] Button clicked!');
        setGameStatus('LOBBY');
        setCurrentQuestion(null);
        setTimeLeft(0);
        setGameResult('');
        setRenderKey(prev => prev + 1);

        // Request fresh room details from server
        if (roomInfo.id !== null && roomInfo.id !== undefined) {
            const textData = `${roomInfo.id}`;
            const encoder = new TextEncoder();
            const stringBytes = encoder.encode(textData);
            const packet = new Uint8Array(1 + stringBytes.length);
            packet[0] = OPS.GET_ROOM_DETAIL;
            packet.set(stringBytes, 1);
            socket.emit("client_to_server", packet);
            console.log('[REPLAY] Requested fresh room details');
        }

        console.log('[REPLAY] State reset to LOBBY');
    };

    // --- ADMIN HANDLERS ---
    const handleGetAllUsers = () => {
        const packet = new Uint8Array(1);
        packet[0] = OPS.GET_ALL_USERS;
        socket.emit("client_to_server", packet);
    };

    const handleDeleteUser = (userId) => {
        const encoder = new TextEncoder();
        const bytes = encoder.encode(userId.toString());
        const packet = new Uint8Array(1 + bytes.length);
        packet[0] = OPS.DELETE_USER;
        packet.set(bytes, 1);
        socket.emit("client_to_server", packet);
    };

    const handleGetUserDetail = (userId) => {
        const encoder = new TextEncoder();
        const bytes = encoder.encode(userId.toString());
        const packet = new Uint8Array(1 + bytes.length);
        packet[0] = OPS.GET_USER_DETAIL;
        packet.set(bytes, 1);
        socket.emit("client_to_server", packet);
    };

    return {
        state: {
            username, setUsername, password, setPassword,
            userId, score, status, userRole,
            gameStatus, setGameStatus, currentQuestion, timeLeft, gameResult, renderKey,
            roomInfo, roomMembers, isHost,
            rooms, leaderboard,
            players, // Live scores during gameplay
            wrongAnswer, setWrongAnswer, // Mode 2: Dimming effect
            idleUsers, incomingInvite,
            setIncomingInvite, // Need this for dismissing manually
            allUsers, selectedUserDetail, setSelectedUserDetail
        },
        actions: {
            handleSubmit,
            handleCreateRoom,
            handleJoinRoom,
            handleLeaveRoom,
            handleLogout,
            handleGetLeaderboard,
            handleStartGame,
            handleAnswer,
            handleGetIdleUsers,
            handleSendInvite,
            handleAcceptInvite,
            handleReplay,
            handleGetAllUsers,
            handleDeleteUser,
            handleGetUserDetail,
        },
        socket
    };
};

