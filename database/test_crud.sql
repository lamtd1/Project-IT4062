-- ===============================
-- BẬT FOREIGN KEY
-- ===============================
PRAGMA foreign_keys = ON;

-- ===============================
-- CLEAN DATA (DELETE trước khi test)
-- ===============================
DELETE FROM questions;
DELETE FROM rooms;
DELETE FROM users;

-- ===============================
-- TEST USERS
-- ===============================

-- CREATE
INSERT INTO users (id, username, created_at)
VALUES
(1, 'hungkao', datetime('now')),
(2, 'admin', datetime('now'));

-- READ
SELECT * FROM users;

-- UPDATE
UPDATE users
SET username = 'HungKaoDev'
WHERE id = 1;

-- READ AGAIN
SELECT * FROM users WHERE id = 1;

-- ===============================
-- TEST ROOMS
-- ===============================

-- CREATE
INSERT INTO rooms (id, name, owner_id, created_at)
VALUES
(1, 'Room 1', 1, datetime('now')),
(2, 'Room Admin', 2, datetime('now'));

-- READ
SELECT * FROM rooms;

-- UPDATE
UPDATE rooms
SET name = 'Updated Room 1'
WHERE id = 1;

-- READ AGAIN
SELECT * FROM rooms WHERE id = 1;

-- ===============================
-- TEST QUESTIONS
-- ===============================

-- CREATE
INSERT INTO questions (id, content, answer, created_at)
VALUES
(1, '2 + 2 = ?', '4', datetime('now')),
(2, 'Capital of Japan?', 'Tokyo', datetime('now'));

-- READ
SELECT * FROM questions;

-- UPDATE
UPDATE questions
SET content = '3 + 3 = ?', answer = '6'
WHERE id = 1;

-- READ AGAIN
SELECT * FROM questions WHERE id = 1;

-- ===============================
-- DELETE TEST
-- ===============================

DELETE FROM questions WHERE id = 2;
DELETE FROM rooms WHERE id = 2;
DELETE FROM users WHERE id = 2;

-- FINAL CHECK
SELECT * FROM users;
SELECT * FROM rooms;
SELECT * FROM questions;
