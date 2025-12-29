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

/**
 * Broadcast current scores to all players (live updates during game)
 * Format: MSG_SCORE_UPDATE (0x25) + "username1:score1,username2:score2,..."
 */
void broadcast_scores(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->player_count == 0) return;
    
    char msg[512];
    msg[0] = MSG_SCORE_UPDATE; // 0x25
    
    // Build payload: "username1:score1,username2:score2,..."
    char payload[500] = "";
    for (int i = 0; i < r->player_count; i++) {
        char temp[64];
        sprintf(temp, "%s:%d,", r->members[i].username, r->members[i].score);
        strcat(payload, temp);
    }
    
    // Remove trailing comma
    int len = strlen(payload);
    if (len > 0 && payload[len - 1] == ',') {
        payload[len - 1] = '\0';
    }
    
    strcpy(msg + 1, payload);
    
    // Send to all players
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].socket_fd > 0) {
            send_with_delimiter(r->members[i].socket_fd, msg, 1 + strlen(msg + 1));
        }
    }
    
    printf("[SCORES] Broadcast to Room %d: %s\n", room_id, payload);
}

/**
 * Reset room state for replay
 * Called after game ends to allow players to start a new game
 */
void reset_room_for_replay(Room *r) {
    // Reset room-level state
    r->status = ROOM_WAITING;
    r->current_question_idx = 0;
    r->game_log[0] = '\0';
    r->end_broadcasted = 0;
    r->game_id = 0; // Will be set on next game
    
    // Reset all player states
    for (int i = 0; i < r->player_count; i++) {
        r->members[i].score = 0;
        r->members[i].is_eliminated = 0;
        r->members[i].has_answered = 0;
        
        // Reset help/lifeline flags
        r->members[i].help_5050_used = 0;
        r->members[i].help_audience_used = 0;
        r->members[i].help_phone_used = 0;
        r->members[i].help_expert_used = 0;
    }
    
    printf("[REPLAY] Room %d reset for replay\n", r->id);
}

// Helper: Broadcast Game End
void broadcast_end_game(int room_id, sqlite3 *db) {
    Room *r = room_get_by_id(room_id);
    if (!r) return;
    
    // We update the database in broadcast_end_game
    if (r->game_mode != MODE_CLASSIC) {
        // Save scores to database for all players (Mode 1 & 2 only)
        for (int i = 0; i < r->player_count; i++) {
            if (r->members[i].user_id > 0 && r->members[i].score > 0) {
                update_user_score(db, r->members[i].user_id, r->members[i].score);
                printf("[DB] Updated score for user %d: +%d points\n", r->members[i].user_id, r->members[i].score);
            }
        }
    }
    
    // SAVE HISTORY
    int winner_id = 0;
    int max_score = -1;
    for(int i=0; i<r->player_count; i++) {
        if(r->members[i].score > max_score) {
            max_score = r->members[i].score;
            winner_id = r->members[i].user_id;
        }
    }
    
    
    // TẤT CẢ MODES: Reset về WAITING để cho phép chơi lại
    // r->status = ROOM_WAITING; // Moved to reset_room_for_replay
    // r->current_question_idx = 0; // Moved to reset_room_for_replay

    // MODE 0 (Classic): Reset and return early (no DB save)
    if (r->game_mode == MODE_CLASSIC) {
        // Gửi thông báo kết thúc
        for (int i = 0; i < r->player_count; i++) {
            if (r->members[i].socket_fd > 0) {
                char msg[128];
                msg[0] = MSG_GAME_END;
                sprintf(msg + 1, "Game ket thuc! Diem: %d", max_score);
                send_with_delimiter(r->members[i].socket_fd, msg, 1 + strlen(msg + 1));
            }
        }
        
        // Reset for replay
        reset_room_for_replay(r);
        return; // Không lưu history
    }
    
    // Save to DB only if Multiplayer (player_count > 1) AND NOT Practice Mode (0)
    if (r->player_count > 1 && r->game_mode != 0) {
        // Save history and get the game_id
        int game_id = save_history(db, r->name, winner_id, r->game_mode, r->game_log);
        r->game_id = game_id; // Store for player stats
        printf("[DB] Game ID %d saved for room %d\n", game_id, r->id);
    }
    
    // === BROADCAST GAME END (BEFORE RESET) ===
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].socket_fd > 0) {
            char msg[128];
            msg[0] = MSG_GAME_END;
            sprintf(msg + 1, "Game Over! Score: %d", r->members[i].score);
            send_with_delimiter(r->members[i].socket_fd, msg, 1 + strlen(msg + 1));
        }
    }
    
    // === UPDATE WIN STATS (After Broadcast, Before Reset) ===
    if (r->player_count > 1 && r->game_mode != 0) {
        int final_winner_id = 0;
        
        // MODE 1: ELIMINATION (Last survivor wins)
        // If multiple survivors (game end naturally), highest score wins.
        if (r->game_mode == MODE_ELIMINATION) {
             int survivors_count = 0;
             int max_survivor_score = -1;
             
             for (int i = 0; i < r->player_count; i++) {
                 if (!r->members[i].is_eliminated) {
                     survivors_count++;
                     if (r->members[i].score > max_survivor_score) {
                         max_survivor_score = r->members[i].score;
                         final_winner_id = r->members[i].user_id;
                     }
                 }
             }
             // If nobody survived, fallback to max_score (winner_id calculated earlier)
             if (survivors_count == 0) final_winner_id = winner_id; 
        } 
        // MODE 2: SPEED ATTACK (Highest score wins)
        else if (r->game_mode == MODE_SCORE_ATTACK) {
            final_winner_id = winner_id; // Already calculated (max score)
        }
        
        // Update DB
        if (final_winner_id != 0) {
            update_user_win(db, final_winner_id);
            printf("[DB] Updated total_win for user ID %d\n", final_winner_id);
        }
        
        // === SAVE PLAYER STATS FOR ALL PLAYERS ===
        for (int i = 0; i < r->player_count; i++) {
            int player_score = r->members[i].score;
            int player_user_id = r->members[i].user_id;
            
            // Save to user_stats with correct game_id
            save_player_stat(db, player_user_id, r->game_id, player_score, 0);
            
            // Update cumulative total_score
            update_user_score(db, player_user_id, player_score);
            
            printf("[DB] Saved stats for user %d: score=%d, game_id=%d\n", 
                   player_user_id, player_score, r->game_id);
        }
    }

    // === RESET ROOM FOR REPLAY ===
    reset_room_for_replay(r);
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

        // Get Score and Role
        int score = get_user_score(db, user_id);
        int role = get_user_role(db, user_id);
        
        // Save role to session
        s->role = role;
        
        response[0] = MSG_LOGIN_SUCCESS; // 0x03
        
        // Gửi: [Op][ID:Score:Role] (String)
        sprintf(response + 1, "%d:%d:%d", user_id, score, role); 
        send_with_delimiter(client_fd, response, 1 + strlen(response+1));
        
        printf("User '%s' logged in (ID: %d, Score: %d, Role: %d)\n", username, user_id, score, role);
        return 1;
    } else {
        response[0] = MSG_LOGIN_FAILED;
        printf("User '%s' login failed\n", username);
        send_with_delimiter(client_fd, response, 1);
        return 0;
    }
}
