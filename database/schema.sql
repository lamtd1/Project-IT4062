-- Enable foreign keys
PRAGMA foreign_keys = ON;

-- Users table
CREATE TABLE IF NOT EXISTS users (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  username TEXT UNIQUE NOT NULL,
  password_hash TEXT NOT NULL,
  is_online INTEGER DEFAULT 0
);

-- Rooms table
CREATE TABLE IF NOT EXISTS rooms (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  owner_id INTEGER NOT NULL,
  status TEXT NOT NULL, -- waiting / playing / closed
  game_mode TEXT NOT NULL, -- single / score / elimination / hotseat
  FOREIGN KEY(owner_id) REFERENCES users(id)
);

-- Room Members table
CREATE TABLE IF NOT EXISTS room_members (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  room_id INTEGER NOT NULL,
  user_id INTEGER NOT NULL,
  joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  left_at DATETIME,
  FOREIGN KEY(room_id) REFERENCES rooms(id),
  FOREIGN KEY(user_id) REFERENCES users(id)
);

-- Games table
CREATE TABLE IF NOT EXISTS games (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  room_id INTEGER NOT NULL,
  game_mode TEXT NOT NULL, -- same as room
  hotseat_user_id INTEGER, -- nullable
  started_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  ended_at DATETIME,
  FOREIGN KEY(room_id) REFERENCES rooms(id),
  FOREIGN KEY(hotseat_user_id) REFERENCES users(id)
);

-- Questions table
CREATE TABLE IF NOT EXISTS questions (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  question_text TEXT NOT NULL,
  ans_a TEXT NOT NULL,
  ans_b TEXT NOT NULL,
  ans_c TEXT NOT NULL,
  ans_d TEXT NOT NULL,
  correct_ans CHAR(1) NOT NULL,
  level INTEGER
);

-- Game History table
CREATE TABLE IF NOT EXISTS game_history (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  game_id INTEGER NOT NULL,
  user_id INTEGER NOT NULL,
  score INTEGER NOT NULL,
  result TEXT, -- win / lose / quit
  duration INTEGER,
  FOREIGN KEY(game_id) REFERENCES games(id),
  FOREIGN KEY(user_id) REFERENCES users(id)
);

-- Game Rounds table
CREATE TABLE IF NOT EXISTS game_rounds (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  game_id INTEGER NOT NULL,
  question_id INTEGER NOT NULL,
  question_order INTEGER NOT NULL,
  FOREIGN KEY(game_id) REFERENCES games(id),
  FOREIGN KEY(question_id) REFERENCES questions(id)
);

-- Game Actions table
CREATE TABLE IF NOT EXISTS game_actions (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  round_id INTEGER NOT NULL,
  user_id INTEGER NOT NULL,
  answer_given CHAR(1),
  is_correct INTEGER, -- BOOLEAN
  answer_time INTEGER,
  time_remaining INTEGER,
  lifeline_used TEXT,
  score_result INTEGER,
  FOREIGN KEY(round_id) REFERENCES game_rounds(id),
  FOREIGN KEY(user_id) REFERENCES users(id)
);
