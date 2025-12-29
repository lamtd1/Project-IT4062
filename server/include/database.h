#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

sqlite3 *db_init(const char* db_path);
void db_close(sqlite3* db);

int verify_user(sqlite3 *db, const char* username, const char* password);
int add_user(sqlite3* db, const char *name, const char *pass);
void get_leaderboard(sqlite3 *db, char *buffer);
int get_user_score(sqlite3 *db, int user_id);
void update_user_score(sqlite3 *db, int id, int score_to_add);
void update_user_win(sqlite3 *db, int id);

// Get user's game history (for replay feature)
void get_user_game_history(sqlite3 *db, int user_id, char *result_buffer, size_t buffer_size);

void get_user(sqlite3 *db);

void get_questions(sqlite3* db, int difficulty, int limit);
void get_questions_by_ids(sqlite3 *db, const char *ids, char *result_buffer, size_t buffer_size);
int save_history(sqlite3 *db, char *room, int winner_id, int game_mode, char *log);
void save_player_stat(sqlite3 *db, int user_id, int game_id, int score, int rank);
void delete_record(sqlite3 *db, char *table, char *condition);

// Admin functions
int get_user_role(sqlite3 *db, int user_id);
int is_user_deleted(sqlite3 *db, int user_id);
void get_all_users_for_admin(sqlite3 *db, char *buffer);
int soft_delete_user(sqlite3 *db, int user_id);
void update_user_win(sqlite3 *db, int id);
void get_user_detail(sqlite3 *db, int user_id, char *buffer);

#endif