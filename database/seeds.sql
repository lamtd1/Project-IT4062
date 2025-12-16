PRAGMA foreign_keys = ON;

BEGIN TRANSACTION;

--------------------------------------------------
-- USERS
--------------------------------------------------
INSERT INTO users (username, password_hash, is_online) VALUES
('alice', 'hash_alice', 1),
('bob', 'hash_bob', 1),
('charlie', 'hash_charlie', 0),
('david', 'hash_david', 0),
('eva', 'hash_eva', 0);

--------------------------------------------------
-- ROOMS
--------------------------------------------------
INSERT INTO rooms (owner_id, status, game_mode) VALUES
(1, 'waiting', 'single'),
(2, 'playing', 'score'),
(3, 'waiting', 'elimination');

--------------------------------------------------
-- ROOM MEMBERS
--------------------------------------------------
INSERT INTO room_members (room_id, user_id) VALUES
(1, 1),
(1, 2),
(2, 2),
(2, 3),
(3, 3),
(3, 4);

--------------------------------------------------
-- GAMES
--------------------------------------------------
INSERT INTO games (room_id, game_mode, hotseat_user_id) VALUES
(2, 'score', NULL),
(3, 'elimination', NULL);

--------------------------------------------------
-- QUESTIONS (NHIỀU CÂU HƠN)
--------------------------------------------------
INSERT INTO questions (question_text, ans_a, ans_b, ans_c, ans_d, correct_ans, level) VALUES
('Thu do cua Viet Nam la gi?', 'Ha Noi', 'Ho Chi Minh', 'Da Nang', 'Can Tho', 'A', 1),
('Con vat nao la chua son lam?', 'Ho', 'Bao', 'Su tu', 'Meo', 'A', 1),
('Dinh nui cao nhat the gioi?', 'Everest', 'Phu Si', 'Fansipan', 'K2', 'A', 1),
('Ai la tac gia Truyen Kieu?', 'Nguyen Du', 'Nguyen Trai', 'Ho Xuan Huong', 'Ba Huyen Thanh Quan', 'A', 1),
('Nuoc nao dong dan nhat the gioi (2023)?', 'An Do', 'Trung Quoc', 'My', 'Indonesia', 'A', 1),

('Ngon ngu lap trinh nao pho bien nhat?', 'C', 'Java', 'Python', 'Assembly', 'C', 2),
('HTTP viet tat cua gi?', 'HyperText Transfer Protocol', 'High Transfer Text Protocol', 'Host Transfer Protocol', 'None', 'A', 2),
('TCP hoat dong o tang nao?', 'Application', 'Transport', 'Network', 'Data Link', 'B', 2),
('SQL dung de lam gi?', 'Lap trinh', 'Quan ly CSDL', 'Thiet ke UI', 'Test', 'B', 2),
('Linux la gi?', 'He dieu hanh', 'Trinh duyet', 'Ngon ngu', 'Phan mem do hoa', 'A', 2),

('Cau truc du lieu nao theo FIFO?', 'Stack', 'Queue', 'Tree', 'Graph', 'B', 3),
('Thuat toan tim kiem nhanh nhat tren mang sap xep?', 'Linear', 'DFS', 'Binary Search', 'BFS', 'C', 3),
('Big O cua Binary Search?', 'O(n)', 'O(log n)', 'O(n^2)', 'O(1)', 'B', 3);

--------------------------------------------------
-- GAME ROUNDS
--------------------------------------------------
INSERT INTO game_rounds (game_id, question_id, question_order) VALUES
(1, 1, 1),
(1, 2, 2),
(1, 3, 3),
(2, 6, 1),
(2, 7, 2),
(2, 8, 3);

--------------------------------------------------
-- GAME ACTIONS
--------------------------------------------------
INSERT INTO game_actions
(round_id, user_id, answer_given, is_correct, answer_time, time_remaining, lifeline_used, score_result)
VALUES
(1, 2, 'A', 1, 5, 15, NULL, 10),
(1, 3, 'B', 0, 7, 13, NULL, 0),
(2, 2, 'A', 1, 4, 16, '50-50', 10),
(3, 2, 'A', 1, 6, 14, NULL, 10);

--------------------------------------------------
-- GAME HISTORY
--------------------------------------------------
INSERT INTO game_history (game_id, user_id, score, result, duration) VALUES
(1, 2, 30, 'win', 120),
(1, 3, 0, 'lose', 120),
(2, 3, 20, 'win', 90);

COMMIT;
