#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/room.h"

// --- KHAI BÁO CÁC HÀM TỪ GAME.C ---
// (Để room.c có thể gọi được các hàm tính toán nội dung trợ giúp)
extern void get_5050_options(int question_id, char *out_str);
extern void get_audience_stats(int question_id, char *out_str);
extern void get_phone_friend_response(int question_id, char *out_str);
extern void get_expert_advice(int question_id, char *out_str);
// ----------------------------------

static Room rooms[MAX_ROOMS];

void room_system_init() {
    for (int i = 0; i < MAX_ROOMS; i++) {
        rooms[i].id = -1; // Đánh dấu phòng trống
        rooms[i].player_count = 0;
        rooms[i].status = ROOM_WAITING;
        pthread_mutex_init(&rooms[i].lock, NULL); // Init mutex cho từng phòng
    }
    game_init(); // Gọi module Game load câu hỏi
}

int room_create(int user_id, char *username, int socket_fd, char *room_name) {
    // Tìm user xem đã ở phòng nào chưa
    if (room_get_by_user(user_id) != NULL) return -1; // Đã ở trong phòng khác

    for (int i = 0; i < MAX_ROOMS; i++) {
        pthread_mutex_lock(&rooms[i].lock);
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

            // [FIX] Khởi tạo trạng thái trợ giúp cho chủ phòng
            rooms[i].members[0].help_5050_used = 0;
            rooms[i].members[0].help_audience_used = 0;
            rooms[i].members[0].help_phone_used = 0;
            rooms[i].members[0].help_expert_used = 0;
            
            pthread_mutex_unlock(&rooms[i].lock);
            printf("[ROOM] Created Room %d: %s by %s\n", i, room_name, username);
            return i;
        }
        pthread_mutex_unlock(&rooms[i].lock);
    }
    return -2; // Full server
}

int room_join(int room_id, int user_id, char *username, int socket_fd) {
    if (room_id < 0 || room_id >= MAX_ROOMS) return -1;
    if (room_get_by_user(user_id) != NULL) return -2; // User đang kẹt ở phòng khác

    Room *r = &rooms[room_id];
    
    pthread_mutex_lock(&r->lock);
    
    if (r->id == -1) { pthread_mutex_unlock(&r->lock); return -1; } // Phòng không tồn tại
    if (r->status != ROOM_WAITING) { pthread_mutex_unlock(&r->lock); return -3; } // Đang chơi
    if (r->player_count >= MAX_PLAYERS_PER_ROOM) { pthread_mutex_unlock(&r->lock); return -4; } // Full

    int idx = r->player_count;
    r->members[idx].user_id = user_id;
    r->members[idx].socket_fd = socket_fd;
    strncpy(r->members[idx].username, username, 50);
    r->members[idx].is_host = 0;
    r->members[idx].score = 0;
    r->members[idx].is_eliminated = 0;

    // Khởi tạo trạng thái trợ giúp
    r->members[idx].help_5050_used = 0;
    r->members[idx].help_audience_used = 0;
    r->members[idx].help_phone_used = 0;  
    r->members[idx].help_expert_used = 0;
    
    r->player_count++;
    
    pthread_mutex_unlock(&r->lock);
    printf("[ROOM] User %s joined Room %d\n", username, room_id);
    return 1;
}

int room_leave(int user_id) {
    Room *r = room_get_by_user(user_id);
    if (!r) return 0;

    pthread_mutex_lock(&r->lock);
    
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
    
    pthread_mutex_unlock(&r->lock);
    printf("[ROOM] User %d left room\n", user_id);
    return 1;
}

int room_start_game(int room_id, int user_id) {
    if (room_id < 0 || room_id >= MAX_ROOMS) return 0;
    Room *r = &rooms[room_id];
    
    pthread_mutex_lock(&r->lock);
    
    // Check quyền Host
    if (r->members[0].user_id != user_id) { pthread_mutex_unlock(&r->lock); return -1; }
    if (r->player_count < 1) { pthread_mutex_unlock(&r->lock); return -2; } // Cần tối thiểu 2 người (để 1 test)

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

    pthread_mutex_unlock(&r->lock);
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

// Hàm này được Thread của Lâm gọi liên tục để kiểm tra timer
int room_update_timer(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->status != ROOM_PLAYING) return 0;

    pthread_mutex_lock(&r->lock);
    
    double elapsed = difftime(time(NULL), r->question_start_time);
    
    if (elapsed > QUESTION_DURATION) {
        // Hết giờ!
        r->current_question_idx++;
        r->question_start_time = time(NULL);
        
        // Check End Game
        if (r->current_question_idx >= 15) {
            r->status = ROOM_FINISHED;
            pthread_mutex_unlock(&r->lock);
            return 2; // Game Over
        }
        
        pthread_mutex_unlock(&r->lock);
        return 1; // Chuyển câu hỏi
    }
    
    pthread_mutex_unlock(&r->lock);
    return 0;
}

// --- HÀM MỚI: XỬ LÝ QUYỀN TRỢ GIÚP ---
// lifeline_type: 1=50:50, 2=KhanGia, 3=GoiDien, 4=TuVan
int room_use_lifeline(int room_id, int user_id, int lifeline_type, char *result_msg) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->status != ROOM_PLAYING) return -1;
    
    pthread_mutex_lock(&r->lock);
    
    RoomMember *p = NULL;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) { p = &r->members[i]; break; }
    }
    
    if (!p) { 
        pthread_mutex_unlock(&r->lock); 
        return -1; 
    }

    int q_id = r->question_ids[r->current_question_idx];

    // --- CASE 1: 50:50 ---
    if (lifeline_type == 1) {
        if (p->help_5050_used) { strcpy(result_msg, "Ban da su dung quyen 50:50 roi!"); goto end; }
        p->help_5050_used = 1;
        get_5050_options(q_id, result_msg);
    } 
    // --- CASE 2: KHÁN GIẢ ---
    else if (lifeline_type == 2) {
        if (p->help_audience_used) { strcpy(result_msg, "Ban da su dung quyen Hoi y kien khan gia roi!"); goto end; }
        p->help_audience_used = 1;
        get_audience_stats(q_id, result_msg);
    }
    // --- CASE 3: GỌI ĐIỆN THOẠI ---
    else if (lifeline_type == 3) {
        if (p->help_phone_used) { strcpy(result_msg, "Ban da su dung quyen Goi dien thoai cho nguoi than roi!"); goto end; }
        p->help_phone_used = 1;
        get_phone_friend_response(q_id, result_msg);
    }
    // --- CASE 4: TƯ VẤN TẠI CHỖ ---
    else if (lifeline_type == 4) {
        if (p->help_expert_used) { strcpy(result_msg, "Ban da su dung quyen To tu van tai cho roi!"); goto end; }
        p->help_expert_used = 1;
        get_expert_advice(q_id, result_msg);
    } else {
        strcpy(result_msg, "Loai tro giup khong hop le.");
    }

end:
    pthread_mutex_unlock(&r->lock);
    return 1;
}