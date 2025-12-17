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

// Helper: Broadcast Game End
void broadcast_end_game(int room_id, sqlite3 *db) {
    Room *r = room_get_by_id(room_id);
    if (!r) return;
    
    // Save scores to database for all players
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id > 0 && r->members[i].score > 0) {
            update_user_score(db, r->members[i].user_id, r->members[i].score);
            printf("[DB] Updated score for user %d: +%d points\n", r->members[i].user_id, r->members[i].score);
        }
    }
    
    // Broadcast game end message to all players
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].socket_fd > 0) {
            char msg[128];
            msg[0] = MSG_GAME_END;
            sprintf(msg + 1, "Game Over! Score: %d", r->members[i].score);
            send_with_delimiter(r->members[i].socket_fd, msg, 1 + strlen(msg + 1));
        }
    }
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
void handle_register(sqlite3 *db, int client_fd, char *payload) {
    char username[50], password[50];
    if (sscanf(payload, "%s %s", username, password) < 2) return;

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
}

// Handle User Login
void handle_login(sqlite3 *db, int client_fd, Session *s, char *payload) {
    char username[50], password[50];
    if (sscanf(payload, "%s %s", username, password) < 2) return;
    char response[64]; 

    if(is_user_online(username)) {
        response[0] = MSG_ALREADY_LOGIN;
        send_with_delimiter(client_fd, response, 1);
        return;
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
    } else {
        response[0] = MSG_LOGIN_FAILED;
        printf("User '%s' login failed\n", username);
        send_with_delimiter(client_fd, response, 1);
    }
}
