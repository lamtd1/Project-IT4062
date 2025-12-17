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

Question all_questions[MAX_QUESTIONS_LOAD];
int total_questions_loaded = 0;

typedef struct {
    Question *q_array;
    int *counUser lamtd answered B. Result: CHINH XAC! Ban dang o muc cau hoi 3. Tien thuong: 600
Client 5 sent OpCode: 22, Payload: D
User lamtd answered D. Result: SAI ROI! Ra ve voi so tien: 0
Client 5 sent OpCode: 22, Payload: B
User lamtd answered B. Result: Ban da bi loai hoac khong trong phong.
Client 5 sent OpCode: 22, Payload: A
User lamtd answered A. Result: Ban da bi loai hoac khong trong phong.
Client 5 sent OpCode: 22, Payload: A
User lamtd answered A. Result: Ban da bi loai hoac khong trong phong.
Client 5 sent OpCode: 22, Payload: B
User lamtd answered B. Result: Ban da bi loai hoac khong trong phong.
Client 1 (fd=5) disconnected.
New connection, socket fd: 5t;
} LoadCtx;

static int load_cb(void *data, int argc, char **argv, char **axColName ) {
    LoadCtx *ctx = (LoadCtx *)data;
    if (*ctx->count >= MAX_QUESTIONS_LOAD) return 1;
    Question *q = &ctx->q_array[*ctx->count];

    q->id = atoi(argv[0]);
    q->difficulty = atoi(argv[1]);
    strncpy(q->content, argv[2] ? argv[2] : "", sizeof(q->content));
    for (int i = 0; i < 4; i++) {
        strncpy(q->options[i], argv[i + 3] ? argv[i + 3] : "", sizeof(q->options[i]));
    }
    strncpy(q->correct_answer, argv[7] ? argv[7] : "", sizeof(q->correct_answer));
    (*ctx->count)++;
    return 0;
}

int game_init(void *db_conn) {
    sqlite3 *db = (sqlite3 *)db_conn;
    total_questions_loaded = 0;
    LoadCtx ctx = {all_questions, &total_questions_loaded};
    
    const char *sql = "SELECT id, difficulty, content, answer_a, answer_b, answer_c, answer_d, correct_answer FROM questions;";
    
    char *err = 0;
    int rc = sqlite3_exec(db, sql, load_cb, &ctx, &err);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return -1;
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
        return 1; // CORRECT
    }
    
    return -1; // WRONG
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

// --- CLASSIC MODE PRIZE LADDER ---
// 15 mốc thưởng tương ứng level 1 - 15
static const int PRIZE_LADDER[15] = {
    200, 400, 600, 1000, 2000,         // Level 1 - 5 (Safe Haven: 2000)
    3000, 6000, 10000, 14000, 22000,   // Level 6 - 10 (Safe Haven: 22000)
    30000, 40000, 60000, 85000, 150000 // Level 11 - 15 (Max)
};

int get_prize_for_level(int level) {
    if (level < 1 || level > 15) return 0;
    return PRIZE_LADDER[level - 1];
}

int calculate_safe_reward(int current_level) {
    // Trả lời sai ở câu hiện tại (đã vượt qua level - 1)
    if (current_level > 10) return PRIZE_LADDER[9]; // Về mốc 10 (22000)
    if (current_level > 5) return PRIZE_LADDER[4];  // Về mốc 5 (2000)
    return 0; // Chưa qua mốc 5 thì về tay trắng
}





