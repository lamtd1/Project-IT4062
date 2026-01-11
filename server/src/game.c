#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "game.h"

// Mảng chứa tất cả câu hỏi được load từ db
Question all_questions[MAX_QUESTIONS_LOAD];
// Đánh số bao nhiêu câu đã lưu - biến phụ này sau dùng nhiều ở các hàm khác
int total_questions_loaded = 0;


// LoadCtx: Mảng câu hỏi + count: để biết load câu bao nhiêu rồi
typedef struct {
    Question *q_array;
    int *count;
} LoadCtx;



// Hàm helper để load hết câu hỏi từ db trc vào mảng 
static int load_cb(void *data, int argc, char **argv, char **axColName ) {
    // ctx: Mảng câu hỏi + count: để biết load câu bao nhiêu rồi
    LoadCtx *ctx = (LoadCtx *)data;
    // Nếu đã load đủ câu hỏi thì dừng
    if (*ctx->count >= MAX_QUESTIONS_LOAD) return 1;
    // Gán đầu mảng câu hỏi vào mảng q
    Question *q = &ctx->q_array[*ctx->count];

    // Bóc id ra
    q->id = atoi(argv[0]);
    // Bóc độ khó ra
    q->difficulty = atoi(argv[1]);
    // Bóc nội dung ra
    strncpy(q->content, argv[2] ? argv[2] : "", sizeof(q->content));
    // Bóc từng câu trả lời ra
    for (int i = 0; i < 4; i++) {
        strncpy(q->options[i], argv[i + 3] ? argv[i + 3] : "", sizeof(q->options[i]));
    }
    // Bóc đáp án đúng ra
    strncpy(q->correct_answer, argv[7] ? argv[7] : "", sizeof(q->correct_answer));
    // Đẩy count lên để đi tiếp câu mới
    (*ctx->count)++;
    return 0;
}

// Bắt đầu game
int game_init(void *db_conn) {
    // Khởi tạo db
    sqlite3 *db = (sqlite3 *)db_conn;
    // Số câu hỏi load = 0
    total_questions_loaded = 0;
    // gán loadctx vào mảng all_question & biến total_question_load để bắt đầu load hết câu hỏi
    LoadCtx ctx = {all_questions, &total_questions_loaded};
    

    const char *sql = "SELECT id, difficulty, content, answer_a, answer_b, answer_c, answer_d, correct_answer FROM questions;";
    
    char *err = 0;
    // Load câu hỏi dùng hàm helper load_cb, lưu vào mảng ctx
    int rc = sqlite3_exec(db, sql, load_cb, &ctx, &err);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    
    printf("[GAME] Successfully loaded %d questions from DB.\n", total_questions_loaded);
    // Load thành công toàn bộ câu hỏi
    return total_questions_loaded;
}

// Trả câu hỏi trong all_questions theo id / ko thì NULL stddef
Question* get_question_by_id(int id) {
    for (int i = 0; i < total_questions_loaded; i++) {
        if (all_questions[i].id == id) return &all_questions[i];
    }
    return NULL;
}

// Helper: Fisher-Yates shuffle để random mảng index
static void shuffle_indices(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

// Load câu hỏi vào phòng từ mảng all_questions[] đã cache sẵn
// Không query database → không block poll loop
int load_room_questions(void *db_conn, Question *room_questions) {
    (void)db_conn; // Không cần dùng db_conn nữa
    
    if (total_questions_loaded == 0) {
        fprintf(stderr, "[GAME] Lỗi: Chưa load câu hỏi! Gọi game_init() trước.\n");
        return -1;
    }
    
    // Tạo mảng index cho từng độ khó
    int easy_indices[MAX_QUESTIONS_LOAD];
    int medium_indices[MAX_QUESTIONS_LOAD];
    int hard_indices[MAX_QUESTIONS_LOAD];
    int easy_count = 0, medium_count = 0, hard_count = 0;
    
    // Phân loại câu hỏi theo độ khó
    for (int i = 0; i < total_questions_loaded; i++) {
        switch (all_questions[i].difficulty) {
            case 1: easy_indices[easy_count++] = i; break;
            case 2: medium_indices[medium_count++] = i; break;
            case 3: hard_indices[hard_count++] = i; break;
        }
    }
    
    // Shuffle từng mảng index để random hóa
    shuffle_indices(easy_indices, easy_count);
    shuffle_indices(medium_indices, medium_count);
    shuffle_indices(hard_indices, hard_count);
    
    int room_count = 0;
    
    // Lấy 5 câu Easy
    int easy_pick = (easy_count < 5) ? easy_count : 5;
    for (int i = 0; i < easy_pick; i++) {
        room_questions[room_count++] = all_questions[easy_indices[i]];
    }
    
    // Lấy 5 câu Medium
    int medium_pick = (medium_count < 5) ? medium_count : 5;
    for (int i = 0; i < medium_pick; i++) {
        room_questions[room_count++] = all_questions[medium_indices[i]];
    }
    
    // Lấy 5 câu Hard
    int hard_pick = (hard_count < 5) ? hard_count : 5;
    for (int i = 0; i < hard_pick; i++) {
        room_questions[room_count++] = all_questions[hard_indices[i]];
    }
    
    printf("[GAME] Loaded %d questions from cache (E:%d M:%d H:%d)\n", 
           room_count, easy_pick, medium_pick, hard_pick);
    return room_count;
}

int calculate_score(Question *q, char *user_ans, double time_taken) {
    if (!q) return 0;

    // So sánh đáp án - ký tự A, B, C, D
    if (strcasecmp(user_ans, q->correct_answer) == 0) {
        if (time_taken > QUESTION_DURATION) return 0; // Quá giờ coi như 0 điểm
        return 1; // CORRECT
    }
    
    return -1; // WRONG
}

// 1. Logic 50:50 
void get_5050_options(Question *q, char *out_str) {
    if (!q) return;

    int correct_idx = q->correct_answer[0] - 'A';
    int random_wrong;
    // random ra đáp án ko cùng index với đáp án đúng -> 0, 1, 2, 3
    do { random_wrong = rand() % 4; } while (random_wrong == correct_idx);

    char c1 = 'A' + (correct_idx < random_wrong ? correct_idx : random_wrong);
    char c2 = 'A' + (correct_idx < random_wrong ? random_wrong : correct_idx);
    sprintf(out_str, "Hệ thống đã loại bỏ 2 phương án sai. Còn lại: %c và %c", c1, c2);
}

// 2. Logic Khán giả
void get_audience_stats(Question *q, char *out_str) {
    if (!q) return;
     int correct_idx = q->correct_answer[0] - 'A';
     int per[4] = {0,0,0,0};
     
     // Correct answer gets highest prob (e.g. 60-80%)
     per[correct_idx] = 50 + rand()%30; 
     int remain = 100 - per[correct_idx];
     
     for(int i=0; i<4; i++) {
         if(i != correct_idx) {
             per[i] = rand() % (remain + 1);
             remain -= per[i];
         }
     }
     // Distribute remaining (if any) to a random wrong answer
     if(remain > 0) {
         int r;
         do { r = rand()%4; } while(r==correct_idx);
         per[r] += remain;
     }
     
     sprintf(out_str, "Khán giả bình chọn: A:%d%%, B:%d%%, C:%d%%, D:%d%%", 
             per[0], per[1], per[2], per[3]);
}

// 3. Logic Gọi người thân - Trả lời tỉ lệ trượt là 20%
void get_phone_friend_response(Question *q, char *out_str) {
    if (!q) return;
    int coin = rand() % 100;
    if (coin < 80) { // 80% correct
        sprintf(out_str, "Người thân: Theo google thì đáp án là %s nhé!", q->correct_answer);
    } else {
        char wrong_ans[2];
        wrong_ans[1] = '\0';
        do { 
            wrong_ans[0] = 'A' + rand() % 4; 
        } while (strcmp(wrong_ans, q->correct_answer) == 0);
        sprintf(out_str, "Người thân: Mình nghĩ là %s hay sao á...", wrong_ans);
    }
}

// 4. Logic Chuyên gia
void get_expert_advice(Question *q, char *out_str) {
    if (!q) return;
    sprintf(out_str, "Chuyên gia: Với kiến thức của tôi, câu trả lời chính xác là %s.", q->correct_answer);
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





