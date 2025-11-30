#ifndef GAME_H
#define GAME_H

#include <time.h>

#define MAX_QUESTIONS_LOAD 100
#define QUESTION_DURATION 20   // Thời gian 20 giây/câu
#define GAME_SCORE_BASE 100    // Điểm cơ bản

typedef struct {
    int id;
    char content[256];
    char options[4][100]; // 4 đáp án
    char correct_answer[5]; // A, B, C, D
} Question;

// Hàm khởi tạo và load dữ liệu câu hỏi
void game_init();

// Load câu hỏi từ file text
int load_questions_from_file(const char *filename);

// Lấy câu hỏi theo ID
Question* get_question_by_id(int id);

// Kiểm tra đáp án (Case insensitive)
// Trả về: Điểm số (nếu đúng), -1 (nếu sai), 0 (nếu hết giờ/lỗi khác)
int calculate_score(int question_id, char *user_ans, double time_taken);

// Helper: Trộn mảng ID câu hỏi để random cho mỗi phòng
void shuffle_questions(int *array, int count);

#endif