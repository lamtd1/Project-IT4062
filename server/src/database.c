#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "../include/database.h"
#include "../include/utils.h"

static sqlite3 *db = NULL;

// Helper to execute simple SQL
static int db_exec(const char *sql) {
    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        log_message(LOG_ERROR, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }
    return SQLITE_OK;
}

static int db_create_tables(void) {
    const char *sql_users =
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT UNIQUE NOT NULL,"
        "  password_hash TEXT NOT NULL,"
        "  is_online INTEGER DEFAULT 0"
        ");";

    const char *sql_rooms =
        "CREATE TABLE IF NOT EXISTS rooms ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  owner_id INTEGER NOT NULL,"
        "  status TEXT NOT NULL,"
        "  game_mode TEXT NOT NULL,"
        "  FOREIGN KEY(owner_id) REFERENCES users(id)"
        ");";

    const char *sql_room_members =
        "CREATE TABLE IF NOT EXISTS room_members ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  room_id INTEGER NOT NULL,"
        "  user_id INTEGER NOT NULL,"
        "  joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  left_at DATETIME,"
        "  FOREIGN KEY(room_id) REFERENCES rooms(id),"
        "  FOREIGN KEY(user_id) REFERENCES users(id)"
        ");";

    const char *sql_games =
        "CREATE TABLE IF NOT EXISTS games ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  room_id INTEGER NOT NULL,"
        "  game_mode TEXT NOT NULL,"
        "  hotseat_user_id INTEGER,"
        "  started_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  ended_at DATETIME,"
        "  FOREIGN KEY(room_id) REFERENCES rooms(id),"
        "  FOREIGN KEY(hotseat_user_id) REFERENCES users(id)"
        ");";

    const char *sql_questions =
        "CREATE TABLE IF NOT EXISTS questions ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  question_text TEXT NOT NULL,"
        "  ans_a TEXT NOT NULL,"
        "  ans_b TEXT NOT NULL,"
        "  ans_c TEXT NOT NULL,"
        "  ans_d TEXT NOT NULL,"
        "  correct_ans CHAR(1) NOT NULL,"
        "  level INTEGER"
        ");";

    const char *sql_game_history =
        "CREATE TABLE IF NOT EXISTS game_history ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  game_id INTEGER NOT NULL,"
        "  user_id INTEGER NOT NULL,"
        "  score INTEGER NOT NULL,"
        "  result TEXT,"
        "  duration INTEGER,"
        "  FOREIGN KEY(game_id) REFERENCES games(id),"
        "  FOREIGN KEY(user_id) REFERENCES users(id)"
        ");";

    const char *sql_game_rounds =
        "CREATE TABLE IF NOT EXISTS game_rounds ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  game_id INTEGER NOT NULL,"
        "  question_id INTEGER NOT NULL,"
        "  question_order INTEGER NOT NULL,"
        "  FOREIGN KEY(game_id) REFERENCES games(id),"
        "  FOREIGN KEY(question_id) REFERENCES questions(id)"
        ");";

    const char *sql_game_actions =
        "CREATE TABLE IF NOT EXISTS game_actions ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  round_id INTEGER NOT NULL,"
        "  user_id INTEGER NOT NULL,"
        "  answer_given CHAR(1),"
        "  is_correct INTEGER,"
        "  answer_time INTEGER,"
        "  time_remaining INTEGER,"
        "  lifeline_used TEXT,"
        "  score_result INTEGER,"
        "  FOREIGN KEY(round_id) REFERENCES game_rounds(id),"
        "  FOREIGN KEY(user_id) REFERENCES users(id)"
        ");";

    int rc;
    rc = db_exec(sql_users); if (rc != SQLITE_OK) return rc;
    rc = db_exec(sql_rooms); if (rc != SQLITE_OK) return rc;
    rc = db_exec(sql_room_members); if (rc != SQLITE_OK) return rc;
    rc = db_exec(sql_games); if (rc != SQLITE_OK) return rc;
    rc = db_exec(sql_questions); if (rc != SQLITE_OK) return rc;
    rc = db_exec(sql_game_history); if (rc != SQLITE_OK) return rc;
    rc = db_exec(sql_game_rounds); if (rc != SQLITE_OK) return rc;
    rc = db_exec(sql_game_actions); if (rc != SQLITE_OK) return rc;

    return SQLITE_OK;
}

int db_init(const char *db_path) {
    int rc = sqlite3_open(db_path, &db);
    if (rc) {
        log_message(LOG_ERROR, "Can't open database: %s", sqlite3_errmsg(db));
        return -1;
    }
    log_message(LOG_INFO, "Opened database successfully: %s", db_path);
    
    // Enable foreign keys
    db_exec("PRAGMA foreign_keys = ON;");

    // Create tables
    if (db_create_tables() != SQLITE_OK) {
        log_message(LOG_ERROR, "Failed to create tables");
        return -1;
    }

    return 0;
}

void db_close() {
    if (db) {
        sqlite3_close(db);
        db = NULL;
        log_message(LOG_INFO, "Database closed.");
    }
}

// --- User Operations ---

int db_register(const char *username, const char *password) {
    const char *sql = "INSERT INTO users (username, password_hash) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        log_message(LOG_ERROR, "Failed to prepare statement: %s", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    int result = 0;
    if (rc == SQLITE_DONE) {
        log_message(LOG_INFO, "User registered: %s", username);
        result = 1;
    } else {
        log_message(LOG_ERROR, "Registration failed for %s: %s", username, sqlite3_errmsg(db));
        result = 0;
    }

    sqlite3_finalize(stmt);
    return result;
}

int db_login(const char *username, const char *password) {
    const char *sql = "SELECT id FROM users WHERE username = ? AND password_hash = ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    int user_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user_id = sqlite3_column_int(stmt, 0);
        
        // Update is_online
        const char *update_sql = "UPDATE users SET is_online = 1 WHERE id = ?;";
        sqlite3_stmt *update_stmt;
        sqlite3_prepare_v2(db, update_sql, -1, &update_stmt, 0);
        sqlite3_bind_int(update_stmt, 1, user_id);
        sqlite3_step(update_stmt);
        sqlite3_finalize(update_stmt);
        
        log_message(LOG_INFO, "User logged in: %s (ID: %d)", username, user_id);
    } else {
        log_message(LOG_WARN, "Login failed for %s", username);
    }

    sqlite3_finalize(stmt);
    return user_id;
}

int db_get_user_id(const char *username) {
    const char *sql = "SELECT id FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    int user_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return user_id;
}

int db_update_user_password(int user_id, const char *new_password) {
    const char *sql = "UPDATE users SET password_hash = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, new_password, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, user_id);

    rc = sqlite3_step(stmt);
    int result = (rc == SQLITE_DONE) ? 0 : -1;
    
    if (result == 0) {
        log_message(LOG_INFO, "Updated password for user ID: %d", user_id);
    } else {
        log_message(LOG_ERROR, "Failed to update password for user ID %d: %s", user_id, sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return result;
}

int db_delete_user(int user_id) {
    const char *sql = "DELETE FROM users WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, user_id);

    rc = sqlite3_step(stmt);
    int result = (rc == SQLITE_DONE) ? 0 : -1;
    
    if (result == 0) {
        log_message(LOG_INFO, "Deleted user ID: %d", user_id);
    } else {
        log_message(LOG_ERROR, "Failed to delete user ID %d: %s", user_id, sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return result;
}

// --- Question Operations ---

int db_add_question(const Question *q) {
    const char *sql = "INSERT INTO questions (question_text, ans_a, ans_b, ans_c, ans_d, correct_ans, level) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        log_message(LOG_ERROR, "Failed to prepare question insert: %s", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, q->question_text, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, q->ans_a, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, q->ans_b, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, q->ans_c, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, q->ans_d, -1, SQLITE_STATIC);
    char char_ans[2] = {q->correct_ans, '\0'};
    sqlite3_bind_text(stmt, 6, char_ans, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, q->level);

    rc = sqlite3_step(stmt);
    int q_id = -1;
    if (rc == SQLITE_DONE) {
        q_id = (int)sqlite3_last_insert_rowid(db);
        log_message(LOG_INFO, "Question added: ID %d", q_id);
    } else {
        log_message(LOG_ERROR, "Failed to add question: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return q_id;
}

int db_update_question(const Question *q) {
    const char *sql = "UPDATE questions SET question_text = ?, ans_a = ?, ans_b = ?, ans_c = ?, ans_d = ?, correct_ans = ?, level = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, q->question_text, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, q->ans_a, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, q->ans_b, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, q->ans_c, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, q->ans_d, -1, SQLITE_STATIC);
    char char_ans[2] = {q->correct_ans, '\0'};
    sqlite3_bind_text(stmt, 6, char_ans, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, q->level);
    sqlite3_bind_int(stmt, 8, q->id);

    rc = sqlite3_step(stmt);
    int result = (rc == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    
    if (result == 0) {
        log_message(LOG_INFO, "Updated question ID: %d", q->id);
    }
    return result;
}

int db_delete_question(int question_id) {
    const char *sql = "DELETE FROM questions WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, question_id);

    rc = sqlite3_step(stmt);
    int result = (rc == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    
    if (result == 0) {
        log_message(LOG_INFO, "Deleted question ID: %d", question_id);
    }
    return result;
}

int db_get_question(int question_id, Question *q) {
    const char *sql = "SELECT id, question_text, ans_a, ans_b, ans_c, ans_d, correct_ans, level FROM questions WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, question_id);

    int result = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        q->id = sqlite3_column_int(stmt, 0);
        strncpy(q->question_text, (const char*)sqlite3_column_text(stmt, 1), sizeof(q->question_text) - 1);
        strncpy(q->ans_a, (const char*)sqlite3_column_text(stmt, 2), sizeof(q->ans_a) - 1);
        strncpy(q->ans_b, (const char*)sqlite3_column_text(stmt, 3), sizeof(q->ans_b) - 1);
        strncpy(q->ans_c, (const char*)sqlite3_column_text(stmt, 4), sizeof(q->ans_c) - 1);
        strncpy(q->ans_d, (const char*)sqlite3_column_text(stmt, 5), sizeof(q->ans_d) - 1);
        const char *ans = (const char*)sqlite3_column_text(stmt, 6);
        q->correct_ans = ans ? ans[0] : ' ';
        q->level = sqlite3_column_int(stmt, 7);
        result = 0;
    }

    sqlite3_finalize(stmt);
    return result;
}

int db_create_room(int owner_id, const char *game_mode) {
    const char *sql = "INSERT INTO rooms (owner_id, status, game_mode) VALUES (?, 'waiting', ?);";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, owner_id);
    sqlite3_bind_text(stmt, 2, game_mode, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    int room_id = -1;
    if (rc == SQLITE_DONE) {
        room_id = (int)sqlite3_last_insert_rowid(db);
        log_message(LOG_INFO, "Room created: ID %d, Owner %d", room_id, owner_id);
    }

    sqlite3_finalize(stmt);
    return room_id;
}

int db_update_room_status(int room_id, const char *status) {
    const char *sql = "UPDATE rooms SET status = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, room_id);

    rc = sqlite3_step(stmt);
    int result = (rc == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return result;
}

// --- Game Operations ---

int db_create_game(int room_id, const char *game_mode) {
    const char *sql = "INSERT INTO games (room_id, game_mode) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, room_id);
    sqlite3_bind_text(stmt, 2, game_mode, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    int game_id = -1;
    if (rc == SQLITE_DONE) {
        game_id = (int)sqlite3_last_insert_rowid(db);
        log_message(LOG_INFO, "Game started: ID %d, Room %d", game_id, room_id);
    }

    sqlite3_finalize(stmt);
    return game_id;
}

int db_end_game(int game_id) {
    const char *sql = "UPDATE games SET ended_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, game_id);

    rc = sqlite3_step(stmt);
    int result = (rc == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return result;
}

// --- Question Operations ---

int db_get_random_questions(int count, int level, Question *questions) {
    const char *sql = "SELECT id, question_text, ans_a, ans_b, ans_c, ans_d, correct_ans, level FROM questions WHERE level = ? ORDER BY RANDOM() LIMIT ?;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        log_message(LOG_WARN, "Failed to prepare level query, trying all: %s", sqlite3_errmsg(db));
        sql = "SELECT id, question_text, ans_a, ans_b, ans_c, ans_d, correct_ans, level FROM questions ORDER BY RANDOM() LIMIT ?;";
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        sqlite3_bind_int(stmt, 1, count);
    } else {
        sqlite3_bind_int(stmt, 1, level);
        sqlite3_bind_int(stmt, 2, count);
    }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < count) {
        questions[i].id = sqlite3_column_int(stmt, 0);
        strncpy(questions[i].question_text, (const char*)sqlite3_column_text(stmt, 1), sizeof(questions[i].question_text) - 1);
        strncpy(questions[i].ans_a, (const char*)sqlite3_column_text(stmt, 2), sizeof(questions[i].ans_a) - 1);
        strncpy(questions[i].ans_b, (const char*)sqlite3_column_text(stmt, 3), sizeof(questions[i].ans_b) - 1);
        strncpy(questions[i].ans_c, (const char*)sqlite3_column_text(stmt, 4), sizeof(questions[i].ans_c) - 1);
        strncpy(questions[i].ans_d, (const char*)sqlite3_column_text(stmt, 5), sizeof(questions[i].ans_d) - 1);
        const char *ans = (const char*)sqlite3_column_text(stmt, 6);
        questions[i].correct_ans = ans ? ans[0] : ' ';
        questions[i].level = sqlite3_column_int(stmt, 7);
        i++;
    }

    sqlite3_finalize(stmt);
    log_message(LOG_INFO, "Fetched %d questions (Level %d)", i, level);
    return i;
}

// --- Round & Action Operations ---

int db_create_round(int game_id, int question_id, int order) {
    const char *sql = "INSERT INTO game_rounds (game_id, question_id, question_order) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, game_id);
    sqlite3_bind_int(stmt, 2, question_id);
    sqlite3_bind_int(stmt, 3, order);

    rc = sqlite3_step(stmt);
    int round_id = -1;
    if (rc == SQLITE_DONE) {
        round_id = (int)sqlite3_last_insert_rowid(db);
    }
    sqlite3_finalize(stmt);
    return round_id;
}

int db_record_action(int round_id, int user_id, char answer, int is_correct, int time_remaining) {
    const char *sql = "INSERT INTO game_actions (round_id, user_id, answer_given, is_correct, time_remaining) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, round_id);
    sqlite3_bind_int(stmt, 2, user_id);
    char ans_str[2] = {answer, '\0'};
    sqlite3_bind_text(stmt, 3, ans_str, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, is_correct);
    sqlite3_bind_int(stmt, 5, time_remaining);

    rc = sqlite3_step(stmt);
    int result = (rc == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return result;
}

int db_save_game_history(int game_id, int user_id, int score, const char *result) {
    const char *sql = "INSERT INTO game_history (game_id, user_id, score, result) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, game_id);
    sqlite3_bind_int(stmt, 2, user_id);
    sqlite3_bind_int(stmt, 3, score);
    sqlite3_bind_text(stmt, 4, result, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    int res = (rc == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return res;
}
