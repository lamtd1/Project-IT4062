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
int game_init(void *db_conn);

// Lấy câu hỏi theo ID
Question* get_question_by_id(int id);

// Kiểm tra đáp án (Case insensitive)
// Trả về: Điểm số (nếu đúng), -1 (nếu sai), 0 (nếu hết giờ/lỗi khác)
int calculate_score(int question_id, char *user_ans, double time_taken);

// Helper: Trộn mảng ID câu hỏi để random cho mỗi phòng
void shuffle_questions(int *array, int count);

// --- CLASSIC MODE LOGIC ---
// Get prize amount for a given level (1-15)
int get_prize_for_level(int level);

// Calculate safe haven reward if answered incorrectly
int calculate_safe_reward(int current_level);

// --- EXTERN FOR ROOM ACCESS ---
extern Question all_questions[MAX_QUESTIONS_LOAD];
extern int total_questions_loaded;

#endif