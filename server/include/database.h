#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include <sqlite3.h>

// User structure
typedef struct {
    int id;
    char username[50];
    char password[50];
    int is_online;
} User;

// Room structure
typedef struct {
    int id;
    int owner_id;
    char status[20];
    char game_mode[20];
} Room;

// Question structure
typedef struct {
    int id;
    char question_text[256];
    char ans_a[100];
    char ans_b[100];
    char ans_c[100];
    char ans_d[100];
    char correct_ans; // 'A', 'B', 'C', 'D'
    int level;
} Question;

// Game Action structure (replaces ReplayEvent)
typedef struct {
    int id;
    int round_id;
    int user_id;
    char answer_given;
    int is_correct;
    int answer_time;
    int time_remaining;
    char lifeline_used[50];
    int score_result;
} GameAction;

// Initialize database
int db_init(const char *db_path);
void db_close();

// User operations
int db_register(const char *username, const char *password);
int db_login(const char *username, const char *password);
int db_get_user_id(const char *username);

// Room operations
int db_create_room(int owner_id, const char *game_mode);
int db_update_room_status(int room_id, const char *status);

// Game operations
int db_create_game(int room_id, const char *game_mode);
int db_end_game(int game_id);

// Question operations
int db_get_random_questions(int count, int level, Question *questions);
int db_add_question(const Question *q);
int db_update_question(const Question *q);
int db_delete_question(int question_id);
int db_get_question(int question_id, Question *q);

// User management operations
int db_update_user_password(int user_id, const char *new_password);
int db_delete_user(int user_id);

// Round & Action operations
int db_create_round(int game_id, int question_id, int order);
int db_record_action(int round_id, int user_id, char answer, int is_correct, int time_remaining);

// History
int db_save_game_history(int game_id, int user_id, int score, const char *result);

#endif // DATABASE_H
