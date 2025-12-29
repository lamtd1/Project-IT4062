-- Bảng người dùng
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    total_win INTEGER DEFAULT 0,
    total_score INTEGER DEFAULT 0,
    role INTEGER DEFAULT 1, -- 0: Admin, 1: User
    is_deleted INTEGER DEFAULT 0 -- 0: Active, 1: Soft deleted
);

-- Bảng câu hỏi
CREATE TABLE IF NOT EXISTS questions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    content TEXT NOT NULL,
    answer_a TEXT NOT NULL,
    answer_b TEXT NOT NULL,
    answer_c TEXT NOT NULL,
    answer_d TEXT NOT NULL,
    correct_answer CHAR(1) NOT NULL,
    difficulty INTEGER NOT NULL -- 1: Dễ, 2: Trung bình, 3: Khó
);

-- Bảng lịch sử ván đấu
CREATE TABLE IF NOT EXISTS game_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    room_name TEXT,
    winner_id INTEGER,
    played_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    game_mode INTEGER, -- 1: Loại dần, 2: Tính điểm
    log_data TEXT, -- Lưu diễn biến ván đấu
    FOREIGN KEY (winner_id) REFERENCES users (id)
);

-- Bảng thống kê chi tiết cho từng người chơi trong mỗi ván
CREATE TABLE IF NOT EXISTS user_stats (
    user_id INTEGER,
    game_id INTEGER,
    score_achieved INTEGER,
    rank INTEGER,
    PRIMARY KEY (user_id, game_id),
    FOREIGN KEY (user_id) REFERENCES users (id),
    FOREIGN KEY (game_id) REFERENCES game_history (id)
);