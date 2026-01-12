#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "network.h"
#include "room.h"
#include "game.h"
#include "protocol.h"
#include "database.h"

#define BUFFER_SIZE 4096
#define QUESTION_DURATION 30

// Helper: Gửi message với đuôi \n để tránh dính message
int send_with_delimiter(int socket_fd, const char *data, size_t len) {
    int ret = send(socket_fd, data, len, 0);
    if (ret >= 0) {
        char delimiter = '\n';
        send(socket_fd, &delimiter, 1, 0);
    }
    return ret;
}

// Phát câu hỏi tới tất cả người chơi trong phòng
void broadcast_question(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r) {
        printf("[ERROR] Room %d not found!\n", room_id);
        return;
    }
    
    Question *q = &r->questions[r->current_question_idx];
    
    // Tạo gói tin: [MSG_QUESTION] + "Level|Content|A|B|C|D|Duration"
    char buf[BUFFER_SIZE];
    buf[0] = MSG_QUESTION;
    sprintf(buf + 1, "%d|%s|%s|%s|%s|%s|%d", 
        r->current_question_idx + 1,
        q->content,
        q->options[0], q->options[1], q->options[2], q->options[3],
        QUESTION_DURATION
    );
    
    // Gửi tới tất cả người chơi chưa bị loại
    for (int i = 0; i < r->player_count; i++) {
        if (!r->members[i].is_eliminated && r->members[i].socket_fd > 0) {
            send_with_delimiter(r->members[i].socket_fd, buf, 1 + strlen(buf + 1));
        }
    }
    printf("Broadcasting Question Level %d to Room %d\n", r->current_question_idx + 1, room_id);
}

// Phát điểm số hiện tại tới tất cả người chơi
// Format: MSG_SCORE_UPDATE (0x25) + "username1:score1,username2:score2,..."
void broadcast_scores(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->player_count == 0) return;
    
    char msg[512];
    msg[0] = MSG_SCORE_UPDATE;
    
    // Tạo chuỗi điểm: "user1:score1,user2:score2,..."
    char payload[500] = "";
    for (int i = 0; i < r->player_count; i++) {
        char temp[64];
        sprintf(temp, "%s:%d,", r->members[i].username, r->members[i].score);
        strcat(payload, temp);
    }
    
    // Xóa dấu phẩy cuối
    int len = strlen(payload);
    if (len > 0 && payload[len - 1] == ',') {
        payload[len - 1] = '\0';
    }
    
    strcpy(msg + 1, payload);
    
    // Gửi tới tất cả người chơi
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].socket_fd > 0) {
            send_with_delimiter(r->members[i].socket_fd, msg, 1 + strlen(msg + 1));
        }
    }
    
    printf("[SCORES] Broadcast to Room %d: %s\n", room_id, payload);
}

// Reset trạng thái phòng để chơi lại
void reset_room_for_replay(Room *r) {
    r->status = ROOM_WAITING;
    r->current_question_idx = 0;
    r->game_log[0] = '\0';
    r->end_broadcasted = 0;
    r->game_id = 0;
    
    // Reset flags - trợ giúp dùng chung (Coop Mode)
    r->shared_help_5050_used = 0;
    r->shared_help_audience_used = 0;
    r->shared_help_phone_used = 0;
    r->shared_help_expert_used = 0;
    
    // Reset trạng thái người chơi
    for (int i = 0; i < r->player_count; i++) {
        r->members[i].score = 0;
        r->members[i].is_eliminated = 0;
        r->members[i].has_answered = 0;
        r->members[i].help_5050_used = 0;
        r->members[i].help_audience_used = 0;
        r->members[i].help_phone_used = 0;
        r->members[i].help_expert_used = 0;
    }
    
    printf("[REPLAY] Room %d reset for replay\n", r->id);
}

// Phát tín hiệu kết thúc game và lưu điểm vào DB
// Quy định logic endgame
void broadcast_end_game(int room_id, sqlite3 *db) {
    Room *r = room_get_by_id(room_id);
    if (!r) return;
    
    // Tìm người thắng (điểm cao nhất)
    int winner_id = 0;
    int max_score = -1;
    for(int i=0; i<r->player_count; i++) {
        if(r->members[i].score > max_score) {
            max_score = r->members[i].score;
            winner_id = r->members[i].user_id;
        }
    }
    
    // MODE 0 (Classic): Không lưu DB, chỉ reset
    if (r->game_mode == MODE_CLASSIC) {
        for (int i = 0; i < r->player_count; i++) {
            if (r->members[i].socket_fd > 0) {
                char msg[128];
                msg[0] = MSG_GAME_END;
                sprintf(msg + 1, "Game ket thuc! Diem: %d", max_score);
                send_with_delimiter(r->members[i].socket_fd, msg, 1 + strlen(msg + 1));
            }
        }
        reset_room_for_replay(r);
        return;
    }
    
    // Mode multiplayer: Lưu lịch sử và điểm
    if (r->player_count > 1) {
        // Lưu vào bảng game_history
        int game_id = save_history(db, r->name, winner_id, r->game_mode, r->game_log);
        r->game_id = game_id;
        printf("[DB] Game ID %d saved for room %d\n", game_id, r->id);
    }
    
    // Phát thông báo kết thúc tới tất cả người chơi
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].socket_fd > 0) {
            char msg[128];
            msg[0] = MSG_GAME_END;
            sprintf(msg + 1, "Game Over! Score: %d", r->members[i].score);
            send_with_delimiter(r->members[i].socket_fd, msg, 1 + strlen(msg + 1));
        }
    }
    
    // Cập nhật thống kê thắng (chỉ multiplayer)
    if (r->player_count > 1) {
        
        // MODE 1: COOP MODE
        if (r->game_mode == MODE_COOP) {
            // Team chỉ thắng khi HOÀN THÀNH 15 câu (current_index > 14)
            // Nếu current_index < 15 nghĩa là team thua giữa chừng
            int team_won = (r->current_question_idx >= 15);
            
            if (team_won) {
                printf("[DB] COOP VICTORY! Updating total_win for all %d players.\n", r->player_count);
                // Update total_win cho TẤT CẢ thành viên trong users
                for(int i=0; i<r->player_count; i++) {
                    update_user_win(db, r->members[i].user_id);
                }
            } else {
                printf("[DB] COOP LOST. No win stats update.\n");
            }
        } 
        
        // MODE 2: SCORE ATTACK (Người cao điểm nhất thắng)
        else if (r->game_mode == MODE_SCORE_ATTACK) {
             // Logic cũ
             int winner_id = 0;
             int max_score = -1;
             for(int i=0; i<r->player_count; i++) {
                 if(r->members[i].score > max_score) {
                     max_score = r->members[i].score;
                     winner_id = r->members[i].user_id;
                 }
             }
            //  Update bảng users
             if (winner_id != 0) {
                 update_user_win(db, winner_id);
                 printf("[DB] Updated total_win for user ID %d\n", winner_id);
             }
        }
        
        // Lưu thống kê cho tất cả người chơi
        for (int i = 0; i < r->player_count; i++) {
            // Lưu thông tin vào user-stats
            save_player_stat(db, r->members[i].user_id, r->game_id, r->members[i].score);
            // Update tổng điểm trong users
            update_user_score(db, r->members[i].user_id, r->members[i].score);
            printf("[DB] Saved stats for user %d: score=%d\n", r->members[i].user_id, r->members[i].score);
        }
    }

    reset_room_for_replay(r);
}

// Kiểm tra user đã online chưa
int is_user_online(char *username) {
    printf("[DEBUG] Checking if user '%s' is online...\n", username);
    for (int i = 0; i < MAX_CLIENTS + 1; i++){
        if (sessions[i].is_logged_in) {
            printf("[DEBUG] Slot %d: %s (online)\n", i, sessions[i].username);
            if (strcmp(sessions[i].username, username) == 0) {
                 printf("[DEBUG] User '%s' found online at slot %d!\n", username, i);
                 return 1;
            }
        }
    }
    printf("[DEBUG] User '%s' is NOT online.\n", username);
    return 0;
}

// Xử lý đăng ký user mới
int handle_register(sqlite3 *db, int client_fd, char *payload) {
    char username[50], password[50];
    if (sscanf(payload, "%s %s", username, password) < 2) return 0;

    char response[1];
    int success = add_user(db, username, password);
    response[0] = success ? MSG_REGISTER_SUCCESS : MSG_REGISTER_FAILED;
    
    send_with_delimiter(client_fd, response, 1);
    printf("User registration %s: %s\n", success ? "success" : "failed", username);
    return success;
}

// Xử lý đăng nhập
int handle_login(sqlite3 *db, int client_fd, Session *s, char *payload) {
    char username[50], password[50];
    if (sscanf(payload, "%s %s", username, password) < 2) return 0;
    
    char response[64]; 
    
    // Kiểm tra đã online chưa - quản lý session - nếu trong mảng session.is_logged_in thì cảnh báo
    if(is_user_online(username)) {
        response[0] = MSG_ALREADY_LOGIN;
        send_with_delimiter(client_fd, response, 1);
        return 0;
    }

    // Xác thực user
    int user_id = verify_user(db, username, password);
    
    // Xác thực thành công 
    if (user_id > 0) {
        s->is_logged_in = 1;
        s->socket_fd = client_fd;
        s->user_id = user_id;
        strcpy(s->username, username);
        s->role = get_user_role(db, user_id);
        
        int score = get_user_score(db, user_id);
        response[0] = MSG_LOGIN_SUCCESS;
        sprintf(response + 1, "%d:%d:%d", user_id, score, s->role); 
        send_with_delimiter(client_fd, response, 1 + strlen(response+1));
        
        printf("User '%s' logged in (ID: %d, Score: %d, Role: %d)\n", username, user_id, score, s->role);
        return 1;
    } else {
        response[0] = MSG_LOGIN_FAILED;
        send_with_delimiter(client_fd, response, 1);
        printf("User '%s' login failed\n", username);
        return 0;
    }
}
