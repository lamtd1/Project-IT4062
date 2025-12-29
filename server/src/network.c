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

// Helper: Send message with delimiter to prevent TCP coalescing
int send_with_delimiter(int socket_fd, const char *data, size_t len) {
    int ret = send(socket_fd, data, len, 0);
    if (ret >= 0) {
        char delimiter = '\n';
        send(socket_fd, &delimiter, 1, 0);
    }
    return ret;
}

// Helper: Broadcast question to room
void broadcast_question(int room_id) {
    printf("[DEBUG] broadcast_question called for room %d\n", room_id);
    Room *r = room_get_by_id(room_id);
    if (!r) {
        printf("[ERROR] Room %d not found!\n", room_id);
        return;
    }
    
    printf("[DEBUG] Room found. current_question_idx = %d\n", r->current_question_idx);
    printf("[DEBUG] Room found. current_question_idx = %d\n", r->current_question_idx);
    
    // Use Question struct directly
    Question *q = &r->questions[r->current_question_idx];
    printf("[DEBUG] Question ID = %d, Content: %s\n", q->id, q->content);
    
    // Packet: [MSG_QUESTION] + "ID:Content:A:B:C:D"
    char buf[BUFFER_SIZE];
    buf[0] = MSG_QUESTION; // 0x21
    
    // Format payload
    sprintf(buf + 1, "%d|%s|%s|%s|%s|%s|%d", 
        r->current_question_idx + 1, // Level
        q->content,
        q->options[0], q->options[1], q->options[2], q->options[3],
        QUESTION_DURATION
    );
    
    printf("[DEBUG] Broadcasting packet: %s\n", buf + 1);
    
    for (int i = 0; i < r->player_count; i++) {
        if (!r->members[i].is_eliminated && r->members[i].socket_fd > 0) {
            printf("[DEBUG] Sending to socket_fd %d\n", r->members[i].socket_fd);
            send_with_delimiter(r->members[i].socket_fd, buf, 1 + strlen(buf + 1));
        }
    }
    printf("Broadcasting Question Level %d to Room %d\n", r->current_question_idx + 1, room_id);
}

// Helper: Broadcast live scores to all players (Mode 1 & 2)
void broadcast_scores(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r) return;
    
    char msg[512];
    msg[0] = MSG_SCORES_UPDATE;
    
    // Format: "username1:score1,username2:score2,..."
    char payload[500] = "";
    for (int i = 0; i < r->player_count; i++) {
        char temp[64];
        sprintf(temp, "%s:%d,", r->members[i].username, r->members[i].score);
        strcat(payload, temp);
    }
    
    // Remove trailing comma
    int len = strlen(payload);
    if (len > 0) payload[len - 1] = '\0';
    
    strcpy(msg + 1, payload);
    
    // Send to all players
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].socket_fd > 0) {
            send_with_delimiter(r->members[i].socket_fd, msg, 1 + strlen(msg + 1));
        }
    }
    
    printf("[SCORES] Broadcast to Room %d: %s\n", room_id, payload);
}

// Helper: Broadcast Game End
void broadcast_end_game(int room_id, sqlite3 *db) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->player_count == 0) return;
    
    // Find winner (highest score)
    int winner_idx = 0;
    int max_score = r->members[0].score;
    for (int i = 1; i < r->player_count; i++) {
        if (r->members[i].score > max_score) {
            max_score = r->members[i].score;
            winner_idx = i;
        }
    }
    
    // MODE 0 (Classic): KHÔNG cập nhật điểm tích lũy vào DB
    // MODE 1 & 2: Only save winner's score to database
    if (r->game_mode != MODE_CLASSIC) {
        if (r->members[winner_idx].user_id > 0 && max_score > 0) {
            update_user_score(db, r->members[winner_idx].user_id, max_score);
            printf("[DB] Winner %s: +%d points\n", r->members[winner_idx].username, max_score);
        }
    }
    
    // Build message with all scores
    char msg[512];
    sprintf(msg, "=== GAME KET THUC ===\n\nBANG XEP HANG:\n");
    
    // List all scores
    for (int i = 0; i < r->player_count; i++) {
        char line[100];
        sprintf(line, "%d. %s: %d diem%s\n", 
                i + 1, 
                r->members[i].username, 
                r->members[i].score,
                (i == winner_idx) ? " [THANG!]" : "");
        strcat(msg, line);
    }
    
    printf("[DEBUG] Base message for room %d:\n%s\n", room_id, msg);
    
    // Send personalized messages to all players
    for (int i = 0; i < r->player_count; i++) {
        char personal_msg[600];
        sprintf(personal_msg, "%s\n%s", 
                msg,
                (i == winner_idx) ? "\nCHUC MUNG! BAN DA CHIEN THANG!" : "\nBAN DA THUA. HAY CO GANG HON!");
        
        printf("[DEBUG] Sending to %s: %s\n", r->members[i].username, personal_msg);
        
        char resp[650];
        resp[0] = MSG_GAME_END;
        strcpy(resp + 1, personal_msg);
        send_with_delimiter(r->members[i].socket_fd, resp, 1 + strlen(personal_msg));
    }
    
    printf("[GAME] Room %d ended. Winner: %s (%d pts)\n", room_id, r->members[winner_idx].username, max_score);
    
    // Keep status as ROOM_FINISHED - don't reset
    // Client will show Replay/Exit UI
}

// Check if user is already online
int is_user_online(char *username) {
    for (int i = 0; i < MAX_CLIENTS + 1; i++){
        if(sessions[i].is_logged_in && strcmp(sessions[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
}

// Handle User Registration
int handle_register(sqlite3 *db, int client_fd, char *payload) {
    char username[50], password[50];
    if (sscanf(payload, "%s %s", username, password) < 2) return 0;

    char response[1];
    int success = add_user(db, username, password);

    if(success) {
        response[0] = MSG_REGISTER_SUCCESS;
        printf("New user registered: %s\n", username);    
    } else {
        response[0] = MSG_REGISTER_FAILED;
        printf("User registration failed for username: %s\n", username);
    }
    send_with_delimiter(client_fd, response, 1);
    return success;
}

// Handle User Login
int handle_login(sqlite3 *db, int client_fd, Session *s, char *payload) {
    char username[50], password[50];
    if (sscanf(payload, "%s %s", username, password) < 2) return 0;
    char response[64]; 

    if(is_user_online(username)) {
        response[0] = MSG_ALREADY_LOGIN;
        send_with_delimiter(client_fd, response, 1);
        return 0;
    }

    int user_id = verify_user(db, username, password);
    if (user_id > 0) {
        s->is_logged_in = 1;
        s->socket_fd = client_fd;
        s->user_id = user_id;
        strcpy(s->username, username);

        // Get Score
        int score = get_user_score(db, user_id);
        
        response[0] = MSG_LOGIN_SUCCESS; // 0x03
        
        // Gửi: [Op][ID:Score] (String)
        sprintf(response + 1, "%d:%d", user_id, score); 
        send_with_delimiter(client_fd, response, 1 + strlen(response+1));
        
        printf("User '%s' logged in (ID: %d, Score: %d)\n", username, user_id, score);
        return 1;
    } else {
        response[0] = MSG_LOGIN_FAILED;
        printf("User '%s' login failed\n", username);
        send_with_delimiter(client_fd, response, 1);
        return 0;
    }
}
