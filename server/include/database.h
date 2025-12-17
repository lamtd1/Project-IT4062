#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

sqlite3 *db_init(const char* db_path);
void db_close(sqlite3* db);

int verify_user(sqlite3 *db, const char* username, const char* password);
int add_user(sqlite3* db, const char *name, const char *pass);
void get_leaderboard(sqlite3 *db, char *buffer);
int get_user_score(sqlite3 *db, int user_id);
void update_user_score(sqlite3 *db, int id, int score);
void get_user(sqlite3 *db);


void get_questions(sqlite3* db, int difficulty, int limit);

void save_history(sqlite3 *db, char *room, int winner_id, int game_mdoe, char *log);
void save_player_stat(sqlite3 *db, int user_id, int game_id, int score, int rank);

void delete_record(sqlite3 *db, char *table, char *condition);

#endif