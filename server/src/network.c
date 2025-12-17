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
    int q_id = r->question_ids[r->current_question_idx];
    printf("[DEBUG] Question ID = %d\n", q_id);
    
    Question *q = get_question_by_id(q_id);
    if (!q) {
        printf("[ERROR] Question %d not found!\n", q_id);
        return;
    }
    
    printf("[DEBUG] Question loaded: %s\n", q->content);

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
