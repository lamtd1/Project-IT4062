#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

// Callback dùng để in dữ liệu cho các hàm SELECT
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for(int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

// Callback riêng để lấy ID khi Verify User
static int id_callback(void *data, int argc, char **argv, char **azColName) {
    int *user_id = (int *)data;
    if (argc > 0 && argv[0]) {
        *user_id = atoi(argv[0]);
    }
    return 0;
}

// --- Connection functions ---

sqlite3* db_init(const char* db_path) {
    sqlite3 *db;
    if (sqlite3_open(db_path, &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

void db_close(sqlite3* db) {
    if (db) {
        sqlite3_close(db);
        printf("Đã đóng kết nối Database.\n");
    }
}

int get_user_score(sqlite3 *db, int user_id) {
    const char *sql = "SELECT total_score FROM users WHERE id = ?;";
    sqlite3_stmt *stmt;
    int score = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            score = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return score;
}

// --- CRUD functions ---

// Trả về user_id (>0) nếu đúng, -1 nếu sai, -2 nếu bị xóa
int verify_user(sqlite3 *db, const char* username, const char* password) {
    char sql[256];
    int user_id = -1;
    sprintf(sql, "SELECT id FROM users WHERE username = '%s' AND password = '%s' AND is_deleted = 0;", username, password);
    sqlite3_exec(db, sql, id_callback, &user_id, NULL);
    return user_id;
}

int add_user(sqlite3* db, const char *name, const char *pass) {
    char sql[256];
    char *err = 0;
    sprintf(sql, "INSERT INTO users (username, password, total_score, role, is_deleted) VALUES ('%s', '%s', 0, 1, 0);", name, pass);
    if (sqlite3_exec(db, sql, 0, 0, &err) == SQLITE_OK) {
        printf("Đã thêm người dùng thành công!\n");
        return 1;
    } else {
        printf("Lỗi thêm người dùng: %s\n", err);
        sqlite3_free(err);
        return 0;
    }
}

void update_user_score(sqlite3 *db, int id, int score) {
    char sql[256];
    char *err = 0;
    sprintf(sql, "UPDATE users SET total_score = total_score + %d WHERE id = %d;", score, id);
    if (sqlite3_exec(db, sql, 0, 0, &err) == SQLITE_OK) {
        printf("Đã cập nhật điểm người dùng.\n");
    } else {
        printf("Lỗi cập nhật điểm: %s\n", err);
        sqlite3_free(err);
    }
}

void get_user(sqlite3 *db) {
    const char *sql = "SELECT * FROM users;";
    printf("--- Danh sách người dùng ---\n");
    sqlite3_exec(db, sql, callback, 0, NULL);
}

void get_questions(sqlite3* db, int difficulty, int limit) {
    char sql[256];
    sprintf(sql, "SELECT * FROM questions WHERE difficulty = %d ORDER BY RANDOM() LIMIT %d;", difficulty, limit);
    printf("--- Danh sách câu hỏi ngẫu nhiên ---\n");
    sqlite3_exec(db, sql, callback, 0, NULL);
}

void save_history(sqlite3 *db, char *room, int winner_id, int game_mode, char *log) {
    char sql[512];
    char *err = 0;
    sprintf(sql, "INSERT INTO game_history (room_name, winner_id, game_mode, log_data) VALUES ('%s', %d, %d, '%s');", room, winner_id, game_mode, log);
    if (sqlite3_exec(db, sql, 0, 0, &err) == SQLITE_OK) {
        printf("Đã lưu lịch sử trận đấu.\n");
    } else {
        printf("Lỗi lưu lịch sử: %s\n", err);
        sqlite3_free(err);
    }
}

void save_player_stat(sqlite3 *db, int user_id, int game_id, int score, int rank) {
    char sql[256];
    char *err = 0;
    sprintf(sql, "INSERT INTO user_stats (user_id, game_id, score_achieved, rank) VALUES (%d, %d, %d, %d);", user_id, game_id, score, rank);
    if (sqlite3_exec(db, sql, 0, 0, &err) == SQLITE_OK) {
        printf("Đã lưu thống kê người chơi.\n");
    } else {
        printf("Lỗi lưu thống kê: %s\n", err);
        sqlite3_free(err);
    }
}

void delete_record(sqlite3 *db, char *table, char *condition) {
    char sql[256];
    char *err = 0;
    sprintf(sql, "DELETE FROM %s WHERE %s;", table, condition);
    if (sqlite3_exec(db, sql, 0, 0, &err) == SQLITE_OK) {
        printf("Đã xóa bản ghi thành công.\n");
    } else {
        printf("Lỗi xóa bản ghi: %s\n", err);
        sqlite3_free(err);
    }
}

void get_leaderboard(sqlite3 *db, char *buffer) {
    const char *sql = "SELECT username, total_score FROM users WHERE is_deleted = 0 AND role = 1 ORDER BY total_score DESC LIMIT 10;";
    sqlite3_stmt *stmt;
    buffer[0] = '\0';

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        int score = sqlite3_column_int(stmt, 1);
        
        char line[64];
        sprintf(line, "%s:%d,", name, score);
        strcat(buffer, line);
    }
    
    // Remove last comma
    int len = strlen(buffer);
    if (len > 0 && buffer[len-1] == ',') buffer[len-1] = '\0';

    sqlite3_finalize(stmt);
}

// ========== ADMIN FUNCTIONS ==========

// Get user role (0=admin, 1=user)
int get_user_role(sqlite3 *db, int user_id) {
    const char *sql = "SELECT role FROM users WHERE id = ?;";
    sqlite3_stmt *stmt;
    int role = -1;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            role = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return role;
}

// Check if user is deleted
int is_user_deleted(sqlite3 *db, int user_id) {
    const char *sql = "SELECT is_deleted FROM users WHERE id = ?;";
    sqlite3_stmt *stmt;
    int deleted = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            deleted = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return deleted;
}

// Get all non-admin users for admin panel
// Format: "id:username:total_win:total_score:is_online,..."
// is_online is determined by checking sessions on server side, we'll pass it as parameter later

void get_all_users_for_admin(sqlite3 *db, char *buffer) {
    const char *sql = "SELECT id, username, total_win, total_score FROM users WHERE role = 1 AND is_deleted = 0 ORDER BY username;";
    sqlite3_stmt *stmt;
    buffer[0] = '\0';

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *username = (const char *)sqlite3_column_text(stmt, 1);
        int total_win = sqlite3_column_int(stmt, 2);
        int total_score = sqlite3_column_int(stmt, 3);
        
        char line[256];
        sprintf(line, "%d:%s:%d:%d,", id, username, total_win, total_score);
        strcat(buffer, line);
    }
    
    // Remove last comma
    int len = strlen(buffer);
    if (len > 0 && buffer[len-1] == ',') buffer[len-1] = '\0';

    sqlite3_finalize(stmt);
}

// Soft delete user (set is_deleted = 1)
int soft_delete_user(sqlite3 *db, int user_id) {
    char sql[256];
    char *err = 0;
    
    // Don't allow deleting admin users
    if (get_user_role(db, user_id) == 0) {
        printf("Cannot delete admin user!\n");
        return 0;
    }
    
    sprintf(sql, "UPDATE users SET is_deleted = 1 WHERE id = %d AND role = 1;", user_id);
    if (sqlite3_exec(db, sql, 0, 0, &err) == SQLITE_OK) {
        printf("User %d soft deleted successfully.\n", user_id);
        return 1;
    } else {
        printf("Error deleting user: %s\n", err);
        sqlite3_free(err);
        return 0;
    }
}

// Get user details for detailed view
// Format: "id:username:total_win:total_score"
void get_user_detail(sqlite3 *db, int user_id, char *buffer) {
    const char *sql = "SELECT id, username, total_win, total_score FROM users WHERE id = ? AND is_deleted = 0;";
    sqlite3_stmt *stmt;
    buffer[0] = '\0';

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *username = (const char *)sqlite3_column_text(stmt, 1);
        int total_win = sqlite3_column_int(stmt, 2);
        int total_score = sqlite3_column_int(stmt, 3);
        
        sprintf(buffer, "%d:%s:%d:%d", id, username, total_win, total_score);
    }

    sqlite3_finalize(stmt);
}