#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include "database.h"
#include "protocol.h"
#include "room.h"
#include "game.h"

// Send message with newline delimiter to prevent TCP coalescing
int send_with_delimiter(int socket_fd, const char *data, size_t len);
void send_question_to_client(int client_fd, Question *q, int level, int duration);
// Broadcast question to all active players in a room
void broadcast_question(int room_id);
// Broadcast current scores to all players in room (live updates)
void broadcast_scores(int room_id);
// Broadcast game end message and save scores to database
void broadcast_end_game(int room_id, sqlite3 *db);
void reset_room_for_replay(Room *r);

// Authentication Handlers
int handle_register(sqlite3 *db, int client_fd, char *payload);
int handle_login(sqlite3 *db, int client_fd, Session *s, char *payload);
int is_user_online(char *username);

// Global sessions array (defined in main.c)
#define MAX_CLIENTS 10
extern Session sessions[MAX_CLIENTS + 1];

#endif // NETWORK_H
