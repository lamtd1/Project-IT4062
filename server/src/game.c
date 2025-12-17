#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "game.h"

// ... (existing code) ... 

// Helper: Trộn mảng ID câu hỏi
void shuffle_questions(int *array, int count) {
    if (count > 1) {
        for (int i = 0; i < count - 1; i++) {
            int j = i + rand() / (RAND_MAX / (count - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

static Question all_questions[MAX_QUESTIONS_LOAD];
static int total_questions_loaded = 0;

typedef struct {
    Question *q_array;
    int *count;
} LoadCtx;

static int load_cb(void *data, int argc, char **argv, char **axColName ) {
    LoadCtx *ctx = (LoadCtx *)data;
    if (*ctx->count >= MAX_QUESTIONS_LOAD) return 1;
    Question *q = &ctx->q_array[*ctx->count];

    q->id = atoi(argv[0]);
    strncpy(q->content, argv[1] ? argv[1] : "", sizeof(q->content));
    for (int i = 0; i < 4; i++) {
        strncpy(q->options[i], argv[i + 2] ? argv[i + 2] : "", sizeof(q->options[i]));
    }
    strncpy(q->correct_answer, argv[6] ? argv[6] : "", sizeof(q->correct_answer));
    (*ctx->count)++;
    return 0;
}

int game_init(sqlite3 *db) {
    total_questions_loaded = 0;
    LoadCtx ctx = {all_questions, &total_questions_loaded};
    
    const char *sql = "SELECT id, content, answer_a, answer_b, answer_c, answer_d, correct_answer FROM questions;";
    
    char *err = 0;
    if (sqlite3_exec(db, sql, load_cb, &ctx, &err) != SQLITE_OK) {
        printf("[GAME] Error loading questions: %s\n", err);
        sqlite3_free(err);
        return 0;
    }
    
    printf("[GAME] Successfully loaded %d questions from DB.\n", total_questions_loaded);
    return total_questions_loaded;
}


Question* get_question_by_id(int id) {
    for (int i = 0; i < total_questions_loaded; i++) {
        if (all_questions[i].id == id) return &all_questions[i];
    }
    return NULL;
}

int calculate_score(int question_id, char *user_ans, double time_taken) {
    Question *q = get_question_by_id(question_id);
    if (!q) return 0;

    // So sánh đáp án không phân biệt hoa thường
    if (strcasecmp(user_ans, q->correct_answer) == 0) {
        if (time_taken > QUESTION_DURATION) return 0; // Quá giờ coi như 0 điểm
        
        // Công thức: Điểm gốc + (Thời gian còn lại * 10)
        int time_bonus = (int)((QUESTION_DURATION - time_taken) * 10);
        if (time_bonus < 0) time_bonus = 0;
        
        return time_bonus + GAME_SCORE_BASE;
    }
    
    return -1; // Sai
}

// 1. Logic 50:50 
void get_5050_options(int question_id, char *out_str) {
    Question *q = get_question_by_id(question_id);
    if (!q) return;

    int correct_idx = q->correct_answer[0] - 'A';
    int random_wrong;
    do { random_wrong = rand() % 4; } while (random_wrong == correct_idx);

    char c1 = 'A' + (correct_idx < random_wrong ? correct_idx : random_wrong);
    char c2 = 'A' + (correct_idx < random_wrong ? random_wrong : correct_idx);
    sprintf(out_str, "Hệ thống đã loại bỏ 2 phương án sai. Còn lại: %c và %c", c1, c2);
}

// 2. Logic Khán giả
void get_audience_stats(int question_id, char *out_str) {
    Question *q = get_question_by_id(question_id);
    if (!q) return;

    int percents[4] = {0};
    int correct_idx = q->correct_answer[0] - 'A';
    
    // Cho đáp án đúng từ 51% -> 80% (để chắc chắn nó là đa số quá bán)
    percents[correct_idx] = 51 + rand() % 30; 
    int remaining = 100 - percents[correct_idx];
    
    // Chia phần còn lại cho 3 sai
    for (int i = 0; i < 4; i++) {
        if (i != correct_idx) {
            if (remaining > 0) {
                int p = rand() % (remaining + 1);
                percents[i] = p;
                remaining -= p;
            }
        }
    }
    // Cộng nốt phần dư vào 1 ô sai bất kỳ
    if (remaining > 0) {
        int r = (correct_idx + 1) % 4;
        percents[r] += remaining;
    }

    sprintf(out_str, "Ket qua binh chon: A:%d%%, B:%d%%, C:%d%%, D:%d%%", 
            percents[0], percents[1], percents[2], percents[3]);
}

// 3. Logic Gọi điện thoại người thân (Cố định + Đáp án đúng)
void get_phone_friend_response(int question_id, char *out_str) {
    Question *q = get_question_by_id(question_id);
    if (!q) return;

    // Giả lập độ trễ suy nghĩ...
    sprintf(out_str, "[Alo... Alo...]\nMinh ha? Cau nay kho day... De bo tra Google nhe...\nA thay roi! Tin bo di, 100%% dap an la %s nhe!", q->correct_answer);
}

// 4. Logic Tổ tư vấn tại chỗ (Giới thiệu + Đáp án đúng)
void get_expert_advice(int question_id, char *out_str) {
    Question *q = get_question_by_id(question_id);
    if (!q) return;

    // Random tên chuyên gia cho sinh động
    char *names[] = {"Bac Giao su Xoay", "Chi Kinh Hong", "Anh Tien si Giay"};
    char *locs[] = {"Ha Noi", "TP.HCM", "Da Nang"};
    
    int r_n = rand() % 3;
    int r_l = rand() % 3;

    sprintf(out_str, "Toi la %s den tu %s.\nTheo kien thuc nhieu nam nghien cuu cua toi thi dap an %s la chinh xac.", 
            names[r_n], locs[r_l], q->correct_answer);
}




