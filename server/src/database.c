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


// Lưu vào game_history
int save_history(sqlite3 *db, char *room, int winner_id, int game_mode, char *log) {
    char sql[8192];
    char *err = 0;
    sprintf(sql, "INSERT INTO game_history (room_name, winner_id, game_mode, log_data) VALUES ('%s', %d, %d, '%s');", room, winner_id, game_mode, log);
    if (sqlite3_exec(db, sql, 0, 0, &err) == SQLITE_OK) {
        int game_id = (int)sqlite3_last_insert_rowid(db);
        printf("Đã lưu lịch sử trận đấu. Game ID: %d\n", game_id);
        return game_id;
    } else {
        printf("Lỗi lưu lịch sử: %s\n", err);
        sqlite3_free(err);
        return 0; // Error
    }
}

void save_player_stat(sqlite3 *db, int user_id, int game_id, int score) {
    char sql[256];
    char *err = 0;
    sprintf(sql, "INSERT INTO user_stats (user_id, game_id, score_achieved) VALUES (%d, %d, %d);", user_id, game_id, score);
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

void update_user_win(sqlite3 *db, int id) {
    char sql[256];
    char *err = 0;
    sprintf(sql, "UPDATE users SET total_win = total_win + 1 WHERE id = %d;", id);
    if (sqlite3_exec(db, sql, 0, 0, &err) != SQLITE_OK) {
        printf("Error updating win count: %s\n", err);
        sqlite3_free(err);
    } else {
        printf("Updated total_win for user %d\n", id);
    }
}

// Get user's game history for replay feature
void get_user_game_history(sqlite3 *db, int user_id, char *result_buffer, size_t buffer_size) {
    result_buffer[0] = '\0'; // Initialize empty
    
    const char *sql = 
        "SELECT gh.id, gh.room_name, gh.winner_id, gh.played_at, "
        "       gh.game_mode, gh.log_data, us.score_achieved "
        "FROM game_history gh "
        "JOIN user_stats us ON gh.id = us.game_id "
        "WHERE us.user_id = ? "
        "ORDER BY gh.played_at DESC "
        "LIMIT 20";
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparing game history query: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    char temp[512];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int game_id = sqlite3_column_int(stmt, 0);
        const char *room_name = (const char *)sqlite3_column_text(stmt, 1);
        int winner_id = sqlite3_column_int(stmt, 2);
        const char *played_at = (const char *)sqlite3_column_text(stmt, 3);
        int game_mode = sqlite3_column_int(stmt, 4);
        const char *log_data = (const char *)sqlite3_column_text(stmt, 5);
        int score = sqlite3_column_int(stmt, 6);
        
        // Format: game_id|room_name|winner_id|timestamp|mode|score|log_data;
        snprintf(temp, sizeof(temp), "%d|%s|%d|%s|%d|%d|%s;",
                 game_id, room_name ? room_name : "", winner_id,
                 played_at ? played_at : "", game_mode, score,
                 log_data ? log_data : "");
        
        // Append to result buffer if there's space
        if (strlen(result_buffer) + strlen(temp) < buffer_size - 1) {
            strcat(result_buffer, temp);
        } else {
            break; // Buffer full
        }
    }
    
    sqlite3_finalize(stmt);
    
    // Remove trailing semicolon
    size_t len = strlen(result_buffer);
    if (len > 0 && result_buffer[len - 1] == ';') {
        result_buffer[len - 1] = '\0';
    }
}

// Get questions by comma-separated IDs for replay
void get_questions_by_ids(sqlite3 *db, const char *ids, char *result_buffer, size_t buffer_size) {
    result_buffer[0] = '\0';
    
    char sql[512];
    snprintf(sql, sizeof(sql), 
        "SELECT id,content,answer_a,answer_b,answer_c,answer_d,correct_answer "
        "FROM questions WHERE id IN (%s)", ids);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("[DB] Error preparing questions query: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    char temp[1024];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *content = (const char *)sqlite3_column_text(stmt, 1);
        const char *ans_a = (const char *)sqlite3_column_text(stmt, 2);
        const char *ans_b = (const char *)sqlite3_column_text(stmt, 3);
        const char *ans_c = (const char *)sqlite3_column_text(stmt, 4);
        const char *ans_d = (const char *)sqlite3_column_text(stmt, 5);
        const char *correct = (const char *)sqlite3_column_text(stmt, 6);
        
        snprintf(temp, sizeof(temp), "%d|%s|%s|%s|%s|%s|%s;",
                 id, 
                 content ? content : "",
                 ans_a ? ans_a : "",
                 ans_b ? ans_b : "",
                 ans_c ? ans_c : "",
                 ans_d ? ans_d : "",
                 correct ? correct : "");
        
        if (strlen(result_buffer) + strlen(temp) < buffer_size - 1) {
            strcat(result_buffer, temp);
        } else {
            break;
        }
    }
    
    sqlite3_finalize(stmt);
}