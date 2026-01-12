#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include "database.h"
#include "protocol.h"
#include "room.h"
#include "game.h"

// Gửi tin nhắn kèm delimiter
int send_with_delimiter(int socket_fd, const char *data, size_t len);

// Phát câu hỏi tới phòng
void broadcast_question(int room_id);

// Phát điểm số hiện tại
void broadcast_scores(int room_id);

// Phát tín hiệu kết thúc game
void broadcast_end_game(int room_id, sqlite3 *db);

// Reset phòng để chơi lại
void reset_room_for_replay(Room *r);

// Xử lý đăng ký/đăng nhập
int handle_register(sqlite3 *db, int client_fd, char *payload);
int handle_login(sqlite3 *db, int client_fd, Session *s, char *payload);
int is_user_online(char *username);

// Mảng sessions toàn cục (định nghĩa trong main.c)
#define MAX_CLIENTS 100
extern Session sessions[MAX_CLIENTS + 1];

#endif // NETWORK_H
