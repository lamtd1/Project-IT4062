#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/room.h"

// --- KHAI BÁO CÁC HÀM TỪ GAME.C ---
extern void get_5050_options(int question_id, char *out_str);
extern void get_audience_stats(int question_id, char *out_str);
extern void get_phone_friend_response(int question_id, char *out_str);
extern void get_expert_advice(int question_id, char *out_str);
// ----------------------------------

static Room rooms[MAX_ROOMS];

void room_system_init(void *db_conn) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        rooms[i].id = -1; // Đánh dấu phòng trống
        rooms[i].player_count = 0;
        rooms[i].status = ROOM_WAITING;
        // pthread_mutex_init removed
    }
    game_init(db_conn); // Gọi module Game load câu hỏi
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
    if (room_get_by_user(user_id) != NULL) return -2; 

    Room *r = &rooms[room_id];
    
    if (r->id == -1) return -1; // Phòng không tồn tại
    if (r->status != ROOM_WAITING) return -3; // Đang chơi
    if (r->player_count >= MAX_PLAYERS_PER_ROOM) return -4; // Full

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
        // Dồn mảng
        for (int i = idx; i < r->player_count - 1; i++) {
            r->members[i] = r->members[i+1];
        }
        r->player_count--;
        
        // Nếu phòng trống -> Reset
        if (r->player_count == 0) {
            r->id = -1;
            r->status = ROOM_WAITING;
        } else if (idx == 0) {
            // Nếu chủ phòng thoát -> chuyển quyền cho người kế
            r->members[0].is_host = 1;
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

    // Random câu hỏi
    shuffle_questions(r->question_ids, 15);
    r->question_start_time = time(NULL);

    printf("[ROOM] Room %d STARTED\n", room_id);
    return 1;
}

Room* room_get_by_id(int room_id) {
    if (room_id >= 0 && room_id < MAX_ROOMS && rooms[room_id].id != -1)
        return &rooms[room_id];
    return NULL;
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

    int q_id = r->question_ids[r->current_question_idx];

    if (lifeline_type == 1) {
        if (p->help_5050_used) { strcpy(result_msg, "Ban da su dung quyen 50:50 roi!"); goto end; }
        p->help_5050_used = 1;
        get_5050_options(q_id, result_msg);
    } 
    else if (lifeline_type == 2) {
        if (p->help_audience_used) { strcpy(result_msg, "Ban da su dung quyen Hoi y kien khan gia roi!"); goto end; }
        p->help_audience_used = 1;
        get_audience_stats(q_id, result_msg);
    }
    else if (lifeline_type == 3) {
        if (p->help_phone_used) { strcpy(result_msg, "Ban da su dung quyen Goi dien thoai cho nguoi than roi!"); goto end; }
        p->help_phone_used = 1;
        get_phone_friend_response(q_id, result_msg);
    }
    else if (lifeline_type == 4) {
        if (p->help_expert_used) { strcpy(result_msg, "Ban da su dung quyen To tu van tai cho roi!"); goto end; }
        p->help_expert_used = 1;
        get_expert_advice(q_id, result_msg);
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