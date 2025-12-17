#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/room.h"

// --- KHAI BÁO THƯ VIỆN ĐỂ SEND ---
#include <sys/socket.h>
#include <unistd.h>
#include "../include/protocol.h"

// --- KHAI BÁO CÁC HÀM TỪ GAME.C ---
extern void get_5050_options(Question *q, char *out_str);
extern void get_audience_stats(Question *q, char *out_str);
extern void get_phone_friend_response(Question *q, char *out_str);
extern void get_expert_advice(Question *q, char *out_str);
extern int load_room_questions(void *db_conn, Question *questions_array);
extern int calculate_score(Question *q, char *answer, double elapsed_time);
extern int get_prize_for_level(int level);
extern int calculate_safe_reward(int level);
// ----------------------------------

static Room rooms[MAX_ROOMS];
static void *global_db = NULL; // Store DB connection

void room_system_init(void *db_conn) {
    global_db = db_conn; // Cache DB connection for room logic
    for (int i = 0; i < MAX_ROOMS; i++) {
        rooms[i].id = -1; // Đánh dấu phòng trống
        rooms[i].player_count = 0;
        rooms[i].status = ROOM_WAITING;
    }
    // game_init(db_conn); // Initial load (optional now, but kept for testing)
}

int room_create(int user_id, char *username, int socket_fd, char *room_name) {
    if (room_get_by_user(user_id) != NULL) return -1; 

    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].id == -1) {
            rooms[i].id = i;
            strncpy(rooms[i].name, room_name, sizeof(rooms[i].name));
            rooms[i].status = ROOM_WAITING;
            rooms[i].player_count = 1;
            
            // Add chủ phòng
            rooms[i].members[0].user_id = user_id;
            rooms[i].members[0].socket_fd = socket_fd;
            strncpy(rooms[i].members[0].username, username, 50);
            rooms[i].members[0].is_host = 1;
            rooms[i].members[0].score = 0;
            rooms[i].members[0].is_eliminated = 0;

            rooms[i].members[0].help_5050_used = 0;
            rooms[i].members[0].help_audience_used = 0;
            rooms[i].members[0].help_phone_used = 0;
            rooms[i].members[0].help_expert_used = 0;
            
            printf("[ROOM] Created Room %d: %s by %s\n", i, room_name, username);
            return i;
        }
    }
    return -2; // Full server
}

int room_join(int room_id, int user_id, char *username, int socket_fd) {
    if (room_id < 0 || room_id >= MAX_ROOMS) return -1;
    Room *r = &rooms[room_id];
    
    if (r->id == -1) return -1; // Room not exist
    if (r->player_count >= MAX_PLAYERS_PER_ROOM) return -2; // Full
    if (r->status != ROOM_WAITING) return -3; // Playing/Finished
    
    // Check if user already in room
    for(int i=0; i<r->player_count; i++) {
        if (r->members[i].user_id == user_id) return -4; 
    }
    
    int idx = r->player_count;
    r->members[idx].user_id = user_id;
    r->members[idx].socket_fd = socket_fd;
    strncpy(r->members[idx].username, username, 50);
    r->members[idx].is_host = 0;
    r->members[idx].score = 0;
    r->members[idx].is_eliminated = 0;

    r->members[idx].help_5050_used = 0;
    r->members[idx].help_audience_used = 0;
    r->members[idx].help_phone_used = 0;  
    r->members[idx].help_expert_used = 0;
    
    r->player_count++;
    
    printf("[ROOM] User %s joined Room %d\n", username, room_id);
    return 1;
}


int room_leave(int user_id) {
    Room *r = room_get_by_user(user_id);
    if (!r) return 0;

    int idx = -1;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) {
            idx = i;
            break;
        }
    }

    if (idx != -1) {
        // Nếu Host thoát -> Kick tất cả
        if (r->members[idx].is_host) {
            printf("[ROOM] Host %s left room %d. Kicking all members.\n", r->members[idx].username, r->id);
            char noti[1];
            noti[0] = MSG_LEAVE_ROOM;
            
            for(int k=0; k < r->player_count; k++) {
                if (k != idx) { // Gửi cho member khác
                     if (r->members[k].socket_fd > 0) {
                         send(r->members[k].socket_fd, noti, 1, 0);
                     }
                }
            }
            // Reset phòng
            r->id = -1;
            r->player_count = 0;
            r->status = ROOM_WAITING;
            return 1;
        }
        
        // Nếu không phải host -> remove bình thường
        for (int i = idx; i < r->player_count - 1; i++) {
            r->members[i] = r->members[i+1];
        }
        r->player_count--;
        
        // Nếu phòng trống -> Reset
        if (r->player_count == 0) {
            r->id = -1;
            r->status = ROOM_WAITING;
        }
    }
    
    printf("[ROOM] User %d left room\n", user_id);
    return 1;
}

int room_start_game(int room_id, int user_id) {
    if (room_id < 0 || room_id >= MAX_ROOMS) return 0;
    Room *r = &rooms[room_id];
    
    // Check quyền Host
    if (r->members[0].user_id != user_id) return -1;
    if (r->player_count < 1) return -2; 

    r->status = ROOM_PLAYING;
    r->current_question_idx = 0;
    
    // Reset điểm & trạng thái
    for(int i=0; i<r->player_count; i++) {
        r->members[i].score = 0;
        r->members[i].is_eliminated = 0;
    }

    // Load 15 questions from DB using ORDER BY RANDOM()
    if (load_room_questions(global_db, r->questions) < 15) {
        printf("[ERROR] Failed to load 15 questions for room %d\n", room_id);
        // Fallback? Or fail? For now, proceed (might crash if array not full)
    }

    r->question_start_time = time(NULL);
    printf("[ROOM] Room %d started game by %d\n", room_id, user_id);
    return 1;
}


Room* room_get_by_id(int room_id) {
    if (room_id < 0 || room_id >= MAX_ROOMS) return NULL;
    if (rooms[room_id].id == -1) return NULL;
    return &rooms[room_id];
}

Room* room_get_by_user(int user_id) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].id != -1) {
            for (int j = 0; j < rooms[i].player_count; j++) {
                if (rooms[i].members[j].user_id == user_id) {
                    return &rooms[i];
                }
            }
        }
    }
    return NULL;
}

int room_update_timer(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->status != ROOM_PLAYING) return 0;

    double elapsed = difftime(time(NULL), r->question_start_time);
    
    if (elapsed > QUESTION_DURATION) {
        // Hết giờ!
        r->current_question_idx++;
        r->question_start_time = time(NULL);
        
        // Check End Game
        if (r->current_question_idx >= 15) {
            r->status = ROOM_FINISHED;
            return 2; // Game Over
        }
        
        return 1; // Chuyển câu hỏi
    }
    
    return 0;
}

int room_use_lifeline(int room_id, int user_id, int lifeline_type, char *result_msg) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->status != ROOM_PLAYING) return -1;
    
    RoomMember *p = NULL;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) { p = &r->members[i]; break; }
    }
    
    if (!p) return -1; 

    // Use Question struct directly
    Question *q = &r->questions[r->current_question_idx];

    if (lifeline_type == 1) {
        if (p->help_5050_used) { strcpy(result_msg, "Ban da su dung quyen 50:50 roi!"); goto end; }
        p->help_5050_used = 1;
        get_5050_options(q, result_msg);
    } 
    else if (lifeline_type == 2) {
        if (p->help_audience_used) { strcpy(result_msg, "Ban da su dung quyen Hoi y kien khan gia roi!"); goto end; }
        p->help_audience_used = 1;
        get_audience_stats(q, result_msg);
    }
    else if (lifeline_type == 3) {
        if (p->help_phone_used) { strcpy(result_msg, "Ban da su dung quyen Goi dien thoai cho nguoi than roi!"); goto end; }
        p->help_phone_used = 1;
        get_phone_friend_response(q, result_msg);
    }
    else if (lifeline_type == 4) {
        if (p->help_expert_used) { strcpy(result_msg, "Ban da su dung quyen To tu van tai cho roi!"); goto end; }
        p->help_expert_used = 1;
        get_expert_advice(q, result_msg);
    } else {
        strcpy(result_msg, "Loai tro giup khong hop le.");
    }

end:
    return 1;
}

// FORMAT: "id-name-count-status,id2-name2-count2-status2"
void room_get_list_string(char *buffer) {
    buffer[0] = '\0';
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].id != -1) {
            char temp[128];
            // ID:Name:Count:Status
            sprintf(temp, "%d:%s:%d:%d,", 
                rooms[i].id, 
                rooms[i].name, 
                rooms[i].player_count, 
                rooms[i].status
            );
            strcat(buffer, temp);
        }
    }
    // Remove last comma
    int list_len = strlen(buffer);
    if (list_len > 0) buffer[list_len - 1] = '\0';
}

// FORMAT: "host_flag:username:score,..."
void room_get_detail_string(int room_id, char *buffer) {
    buffer[0] = '\0';
    Room *r = room_get_by_id(room_id);
    if (!r) return;
    
    for (int i = 0; i < r->player_count; i++) {
        char temp[128];
        sprintf(temp, "%d:%s:%d,", 
            r->members[i].is_host,
            r->members[i].username,
            r->members[i].score
        );
        strcat(buffer, temp);
    }
    
    int len = strlen(buffer);
    if (len > 0) buffer[len - 1] = '\0';
}

int room_handle_answer(int user_id, char *answer, char *result_msg) {
    Room *r = room_get_by_user(user_id);
    if (!r || r->status != ROOM_PLAYING) {
        strcpy(result_msg, "Loi: Phong khong choi hoac ban chua vao phong.");
        return 0; // Error
    }

    int idx = -1;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) { idx = i; break; }
    }
    if (idx == -1 || r->members[idx].is_eliminated) {
        strcpy(result_msg, "Ban da bi loai hoac khong trong phong.");
        return 0; 
    }

    // Use Question struct directly
    Question *q = &r->questions[r->current_question_idx];
    double elapsed = difftime(time(NULL), r->question_start_time);
    
    // Check answer via Game module
    // Note: calculate_score now returns 1 (correct), -1 (wrong), 0 (timeout)
    int res = calculate_score(q, answer, elapsed);
    
    // Current Level: 1..15
    int current_level = r->current_question_idx + 1; 

    if (res == 1) {
        // CORRECT
        int prize = get_prize_for_level(current_level);
        r->members[idx].score = prize;
        sprintf(result_msg, "CHINH XAC! Ban dang o muc cau hoi %d. Tien thuong: %d", current_level, prize);
        
        // Logic: Nếu đây là mode Single Player (hoặc Multiplayer trả lời song song), 
        // ta không cần đợi hết giờ mới qua câu. Nhưng để đồng bộ Multiplayer, ta thường đợi Timer.
        // Tạm thời Logic hiện tại: User trả lời xong -> Gửi kết quả cho User đó -> Server vẫn đợi Timer hết để chuyển câu (Cho phép người khác trả lời).
        // Nếu muốn chuyển ngay khi tất cả đã trả lời: Cần check thêm.
        
        return 1;
    } else {
        // WRONG or TIMEOUT
        int safe_prize = calculate_safe_reward(current_level);
        r->members[idx].score = safe_prize;
        r->members[idx].is_eliminated = 1; // Bị loại khỏi vòng chiến đấu tiếp theo
        
        if (res == 0) sprintf(result_msg, "HET GIO! Ra ve voi so tien: %d", safe_prize);
        else sprintf(result_msg, "SAI ROI! Ra ve voi so tien: %d", safe_prize);
        
        return 2; // Eliminated
    }
}

int room_walk_away(int user_id, char *result_msg) {
    Room *r = room_get_by_user(user_id);
    if (!r || r->status != ROOM_PLAYING) return 0;
    
    int idx = -1;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) { idx = i; break; }
    }
    if (idx == -1 || r->members[idx].is_eliminated) return 0;

    // Dừng cuộc chơi: Giữ nguyên điểm số hiện tại
    r->members[idx].is_eliminated = 1; // Coi như dừng, không tham gia câu sau
    sprintf(result_msg, "Ban da dung cuoc choi. Tong tien thuong: %d", r->members[idx].score);
    
    return 1;
}