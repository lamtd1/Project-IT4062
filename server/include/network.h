#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include "database.h"

// Send message with newline delimiter to prevent TCP coalescing
int send_with_delimiter(int socket_fd, const char *data, size_t len);

// Broadcast question to all active players in a room
void broadcast_question(int room_id);

// Broadcast game end message and save scores to database
void broadcast_end_game(int room_id, sqlite3 *db);

#endif // NETWORK_H
