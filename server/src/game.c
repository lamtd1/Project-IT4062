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

// RoomLoadCtx: Mảng câu hỏi + current_count: để biết load câu bao nhiêu rồi
// limit: giới hạn 15 câu mỗi phòng - thay đổi đc

typedef struct {
    Question *target_array;
    int *current_count;
    int limit;
} RoomLoadCtx;

// Hàm helper để load câu hỏi vào mảng target_array - cho từng phòng load 15 câu hỏi
static int room_load_cb(void *data, int argc, char **argv, char **axColName ) {
    RoomLoadCtx *ctx = (RoomLoadCtx *)data;
    // Load đủ 15 câu thì dừng
    if (*ctx->current_count >= ctx->limit) return 1;
    
    // Gán đầu mảng câu hỏi vào biến q
    Question *q = &ctx->target_array[*ctx->current_count];
    
    // Tách id ra
    q->id = atoi(argv[0]);
    // Tách độ khó ra
    q->difficulty = atoi(argv[1]);
    // Tách câu hỏi ra
    strncpy(q->content, argv[2] ? argv[2] : "", sizeof(q->content));
    // Tách từng câu trả lời và nội dung
    for (int i = 0; i < 4; i++) {
        strncpy(q->options[i], argv[i + 3] ? argv[i + 3] : "", sizeof(q->options[i]));
    }
    // Tác đáp án đúng ra
    strncpy(q->correct_answer, argv[7] ? argv[7] : "", sizeof(q->correct_answer));
    // Dịch lên đáp án tiếp theo
    (*ctx->current_count)++;
    return 0;
}

// Load câu hỏi vào các phòng từ database - lấy random trực tiếp luôn
int load_room_questions(void *db_conn, Question *room_questions) {
    sqlite3 *db = (sqlite3 *)db_conn;
    int count = 0;
    char *err = 0;
    
    // 5 Easy (Diff 1)
    RoomLoadCtx ctx1 = {room_questions, &count, 15};
    char sql1[512];
    sprintf(sql1, "SELECT id, difficulty, content, answer_a, answer_b, answer_c, answer_d, correct_answer FROM questions WHERE difficulty = 1 ORDER BY RANDOM() LIMIT 5;");
    if (sqlite3_exec(db, sql1, room_load_cb, &ctx1, &err) != SQLITE_OK) {
         fprintf(stderr, "SQL error (Easy): %s\n", err); sqlite3_free(err); return -1;
    }

    // 5 Medium (Diff 2)
    RoomLoadCtx ctx2 = {room_questions, &count, 15};
    char sql2[512];
    sprintf(sql2, "SELECT id, difficulty, content, answer_a, answer_b, answer_c, answer_d, correct_answer FROM questions WHERE difficulty = 2 ORDER BY RANDOM() LIMIT 5;");
    if (sqlite3_exec(db, sql2, room_load_cb, &ctx2, &err) != SQLITE_OK) {
         fprintf(stderr, "SQL error (Medium): %s\n", err); sqlite3_free(err); return -1;
    }

    // 5 Hard (Diff 3)
    RoomLoadCtx ctx3 = {room_questions, &count, 15};
    char sql3[512];
    sprintf(sql3, "SELECT id, difficulty, content, answer_a, answer_b, answer_c, answer_d, correct_answer FROM questions WHERE difficulty = 3 ORDER BY RANDOM() LIMIT 5;");
    if (sqlite3_exec(db, sql3, room_load_cb, &ctx3, &err) != SQLITE_OK) {
         fprintf(stderr, "SQL error (Hard): %s\n", err); sqlite3_free(err); return -1;
    }
    
    printf("[GAME] Loaded %d questions for room.\n", count);
    return count;
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





