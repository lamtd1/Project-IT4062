#include <stdio.h>
#include <stdlib.h>
#include "database.h"

// --- Utils functions ---

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   for(int i = 0; i < argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

void db_query(sqlite3 *db, const char *sql) {
    char *err = 0;
    int rc = sqlite3_exec(db, sql, callback, 0, &err); 
    if (rc != SQLITE_OK) {
        printf("Lỗi truy vấn: %s\n", err);
        sqlite3_free(err);
    }
}

void db_execute(sqlite3 *db, const char *sql, const char *success_msg) {
    char *err = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err); 
    if (rc != SQLITE_OK) {
        printf("Lỗi thực thi: %s\n", err);
        sqlite3_free(err);
    } else {
        if (success_msg) printf("%s\n", success_msg);
    }
}

// --- Connection functions ---

sqlite3* db_init(const char* db_path){
    sqlite3 *db;
    if (sqlite3_open(db_path, &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

void db_close(sqlite3* db){
    if (db) {
        sqlite3_close(db); // Sửa lại từ sqlite3_free thành sqlite3_close
        printf("Đã đóng kết nối Database.\n");
    }
}

// --- CRUD functions ---

// User
void add_user(sqlite3* db, const char *name, const char *pass) {
    char sql[256];
    sprintf(sql, "INSERT INTO users (username, password) VALUES ('%s', '%s');", name, pass);
    db_execute(db, sql, "Đã thêm người dùng thành công!");
}

void update_user_score(sqlite3 *db, int id, int score) {
    char sql[256];
    sprintf(sql, "UPDATE users SET total_score = %d WHERE id = %d;", score, id);
    db_execute(db, sql, "Đã cập nhật điểm người dùng.");
}

void get_user(sqlite3 *db) {
    const char *sql = "SELECT * FROM users;";
    printf("--- Danh sách người dùng ---\n");
    db_query(db, sql);
}

// Questions
void get_questions(sqlite3* db, int difficulty, int limit) {
    char sql[256];
    sprintf(sql, "SELECT * FROM questions WHERE difficulty = %d ORDER BY RANDOM() LIMIT %d;", difficulty, limit);
    printf("--- Danh sách câu hỏi ngẫu nhiên ---\n");
    db_query(db, sql);
}

// History & Stats
void save_history(sqlite3 *db, char *room, int winner_id, int game_mode, char *log) {
    char sql[512];
    sprintf(sql, "INSERT INTO game_history (room_name, winner_id, game_mode, log_data) VALUES ('%s', %d, %d, '%s');", room, winner_id, game_mode, log);
    db_execute(db, sql, "Đã lưu lịch sử trận đấu.");
}

void save_player_stat(sqlite3 *db, int user_id, int game_id, int score, int rank) {
    char sql[256];
    sprintf(sql, "INSERT INTO user_stats (user_id, game_id, score_achieved, rank) VALUES (%d, %d, %d, %d);", user_id, game_id, score, rank);
    db_execute(db, sql, "Đã lưu thống kê người chơi.");
}

// General Delete
void delete_record(sqlite3 *db, char *table, char *condition) {
    char sql[256];
    sprintf(sql, "DELETE FROM %s WHERE %s;", table, condition);
    db_execute(db, sql, "Đã xóa bản ghi thành công.");
}