#ifndef GAME_H
#define GAME_H

#include <time.h>

#define MAX_QUESTIONS_LOAD 100
#define QUESTION_DURATION 30  // seconds

// Cấu trúc câu hỏi
typedef struct {
    int id;
    int difficulty;  // 1=Easy, 2=Medium, 3=Hard
    char content[512];
    char options[4][256];
    char correct_answer[2];
} Question;

// Khởi tạo game (load câu hỏi)
// Helper: Load 15 questions for a specific room (SQL RANDOM)
int load_room_questions(void *db_conn, Question *room_questions);

// Khởi tạo game (load câu hỏi - giữ lại để test hoặc bỏ)
// int game_init(void *db_conn);

// Kiểm tra đáp án (Case insensitive)
int calculate_score(Question *q, char *user_ans, double time_taken);

// --- HELPER FUNCTIONS (Refactored to take Question*) ---
void get_5050_options(Question *q, char *out_str);
void get_audience_stats(Question *q, char *out_str);
void get_phone_friend_response(Question *q, char *out_str);
void get_expert_advice(Question *q, char *out_str);

// --- CLASSIC MODE LOGIC ---
// Get prize amount for a given level (1-15)
int get_prize_for_level(int level);

// Calculate safe haven reward if answered incorrectly
int calculate_safe_reward(int current_level);

// --- EXTERN FOR ROOM ACCESS ---
extern Question all_questions[MAX_QUESTIONS_LOAD];
extern int total_questions_loaded;

#endif