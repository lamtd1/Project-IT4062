#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <game.h>

static Question all_questions[MAX_QUESTIONS_LOAD];
static int total_questions_loaded = 0;

void game_init() {
    // Đường dẫn tương đối tính từ file thực thi (thường chạy ở root server)
    if (load_questions_from_file("data/questions.txt") == 0) {
        printf("[GAME] Warning: No questions loaded!\n");
    }
}

int load_questions_from_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("[GAME] Error opening questions file");
        return 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), f) && total_questions_loaded < MAX_QUESTIONS_LOAD) {
        // Format: Question|A|B|C|D|CorrectAns
        char *token = strtok(line, "|");
        if (!token) continue;

        Question *q = &all_questions[total_questions_loaded];
        q->id = total_questions_loaded;
        strncpy(q->content, token, sizeof(q->content));

        for (int i = 0; i < 4; i++) {
            token = strtok(NULL, "|");
            if (token) strncpy(q->options[i], token, sizeof(q->options[i]));
        }

        token = strtok(NULL, "|");
        if (token) {
            // Xóa ký tự xuống dòng thừa
            token[strcspn(token, "\r\n")] = 0;
            strncpy(q->correct_answer, token, sizeof(q->correct_answer));
        }
        total_questions_loaded++;
    }
    fclose(f);
    printf("[GAME] Loaded %d questions.\n", total_questions_loaded);
    return total_questions_loaded;
}

Question* get_question_by_id(int id) {
    if (id < 0 || id >= total_questions_loaded) return NULL;
    return &all_questions[id];
}

int calculate_score(int question_id, char *user_ans, double time_taken) {
    Question *q = get_question_by_id(question_id);
    if (!q) return 0;

    // So sánh đáp án không phân biệt hoa thường
    if (strcasecmp(user_ans, q->correct_answer) == 0) {
        if (time_taken > QUESTION_DURATION) return 0; // Quá giờ coi như 0 điểm
        
        // Công thức: Điểm gốc + (Thời gian còn lại * 10)
        // int time_bonus = (int)((QUESTION_DURATION - time_taken) * 10);
        // if (time_bonus < 0) time_bonus = 0;
        
        return GAME_SCORE_BASE;
    }
    
    return -1; // Sai
}

// 1. Logic 50:50 
void get_5050_options(int question_id, char *out_str) {
    Question *q = get_question_by_id(question_id);
    if (!q) return;

    int correct_idx = -1;
    if (strcasecmp(q->correct_answer, "A") == 0) correct_idx = 0;
    else if (strcasecmp(q->correct_answer, "B") == 0) correct_idx = 1;
    else if (strcasecmp(q->correct_answer, "C") == 0) correct_idx = 2;
    else if (strcasecmp(q->correct_answer, "D") == 0) correct_idx = 3;

    int random_wrong;
    do { random_wrong = rand() % 4; } while (random_wrong == correct_idx);

    char c1 = 'A' + (correct_idx < random_wrong ? correct_idx : random_wrong);
    char c2 = 'A' + (correct_idx < random_wrong ? random_wrong : correct_idx);
    sprintf(out_str, "May tinh da loai bo 2 phuong an sai. Con lai: %c & %c", c1, c2);
}

// 2. Logic Khán giả
void get_audience_stats(int question_id, char *out_str) {
    Question *q = get_question_by_id(question_id);
    if (!q) return;

    int percents[4] = {0};
    int correct_idx = 0;
    if (strcasecmp(q->correct_answer, "B") == 0) correct_idx = 1;
    else if (strcasecmp(q->correct_answer, "C") == 0) correct_idx = 2;
    else if (strcasecmp(q->correct_answer, "D") == 0) correct_idx = 3;

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

void shuffle_questions(int *array, int count) {
    for (int i = 0; i < count; i++) {
        array[i] = i % total_questions_loaded; // Fallback nếu ít câu hỏi
    }
    // Fisher-Yates shuffle
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}