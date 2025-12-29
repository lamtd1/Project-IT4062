#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include "database.h"
#include "room.h"
#include "game.h"
#include "protocol.h"
#include "network.h"

#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10 
#define PORT 8080

// Define alias for consistency
#define MSG_REGISTER_FAIL MSG_REGISTER_FAILED

Session sessions[MAX_CLIENTS + 1];  // +1 for slot[0] is server

// Hàm khởi tạo session
void init_session() {
    for (int i = 0; i < MAX_CLIENTS + 1; i++){
        sessions[i].socket_fd = -1;
        sessions[i].is_logged_in = 0;
        sessions[i].user_id = -1;
        strcpy(sessions[i].username, "");
    }
}

// Hàm ghi lịch sử vào file
void log_history(const char *format, ...) {
    va_list args;
    va_start(args, format);
    FILE *fp = fopen("../../logs/server_history.txt", "a");
    if (fp) {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strlen(time_str)-1] = '\0'; // remove newline
        fprintf(fp, "[%s] ", time_str);
        vfprintf(fp, format, args);
        fprintf(fp, "\n");
        fclose(fp);
    }
    va_end(args);
}


void handle_get_online_users(int client_fd, struct pollfd* fds) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    buffer[0] = MSG_ONLINE_USERS_RESULT;
    char *list_ptr = buffer + 1; 
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {

        if (fds[i].fd != -1 && fds[i].fd != client_fd && sessions[i].is_logged_in == 1) {
            strcat(list_ptr, sessions[i].username);
            strcat(list_ptr, ","); 
        }
    }
    int len = strlen(list_ptr);
    if (len > 0 && list_ptr[len - 1] == ',') {
        list_ptr[len - 1] = '\0';
    }
    printf("Sending online list to %d: %s\n", client_fd, list_ptr);

    // Gửi về client: [0x08] + "user1,user2,user3"
    send_with_delimiter(client_fd, buffer, 1 + strlen(list_ptr));
}

void handle_get_leaderboard(sqlite3 *db, int client_fd) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    char *list_ptr = buffer + 1;

    get_leaderboard(db, list_ptr);
    buffer[0] = MSG_LEADERBOARD_LIST;
    send_with_delimiter(client_fd, buffer, 1 + strlen(list_ptr));
}

void handle_get_idle_users(int client_fd, struct pollfd* fds) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    buffer[0] = MSG_IDLE_USERS_LIST; // 0x43
    char *list_ptr = buffer + 1; 
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {
        // Logged in AND NOT in a room (room_get_by_user returns NULL)
        // Also exclude self
        if (fds[i].fd != -1 && fds[i].fd != client_fd && sessions[i].is_logged_in == 1) {
            if (room_get_by_user(sessions[i].user_id) == NULL) {
                strcat(list_ptr, sessions[i].username);
                strcat(list_ptr, ","); 
            }
        }
    }
    int len = strlen(list_ptr);
    if (len > 0 && list_ptr[len - 1] == ',') {
        list_ptr[len - 1] = '\0';
    }
    // printf("Sending idle list to %d: %s\n", client_fd, list_ptr);
    send_with_delimiter(client_fd, buffer, 1 + strlen(list_ptr));
}

void handle_invite_friend(int sender_fd, int sender_id, char* sender_name, char* target_name, struct pollfd* fds) {
    // 1. Find Sender's Room
    Room *r = room_get_by_user(sender_id);
    if (!r) return; // Sender not in room

    // 2. Find Target Socket
    int target_fd = -1;
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {
        if (sessions[i].is_logged_in && strcmp(sessions[i].username, target_name) == 0) {
            target_fd = fds[i].fd;
            break;
        }
    }

    if (target_fd != -1) {
        // 3. Send Invite: [0x48] "SenderName:RoomID:RoomName"
        char buffer[BUFFER_SIZE];
        buffer[0] = MSG_INVITE_RECEIVED; // 0x48
        
        // Format payload: SenderName:RoomID:RoomName
        // Note: RoomName might contain special formatted chars from our previous task (Name:Mode), so just send ID is safest, but Name is nice.
        // Let's send: SenderName:RoomID
        sprintf(buffer + 1, "%s:%d", sender_name, r->id);
        
        send_with_delimiter(target_fd, buffer, 1 + strlen(buffer + 1));
        printf("User %s invited %s to room %d\n", sender_name, target_name, r->id);
    } else {
        printf("Invite failed: User %s not found or offline\n", target_name);
    }
}

int main(){
    sqlite3 *db = db_init("../../database/database.db");
    if(!db) {
        fprintf(stderr, "Failed to connect to database\n");
        return -1;
    }

    init_session();
    room_system_init(db);


    char buffer[BUFFER_SIZE];
    int server_fd, new_socket, i;
    struct sockaddr_in address;
    int opt = 1;

    // Mảng chứa các file descriptor (fd) cần theo dõi
    struct pollfd fds[MAX_CLIENTS + 1];

    // Tạo socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("SOCKET FAILED");
        exit(EXIT_FAILURE);
    }

    // Cấu hình socket, dùng lại cổng
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        perror("SETSOCKOPT FAIL");
        exit(EXIT_FAILURE);
    }

    // Gán địa chỉ IP & PORT 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0){
        perror("BIND FAIL");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 3) < 0){
        perror("LISTEND FAIL");
        exit(EXIT_FAILURE);
    }

    printf("LISTENING ON PORT %d\n", PORT);

    // Slot[0] là server
    fds[0].fd = server_fd;
    fds[0].events = POLLIN; // Chỉ active khi có dữ liệu đọc

    // Khởi tạo slot trống là -1
    for (i = 1; i < MAX_CLIENTS + 1; i++){
        fds[i].fd = -1;
    }

    while(1){
        // Block đến khi có 1 sự kiện xảy ra HOẶC hết timeout (100ms)
        // Timeout để update timer game
        int ready = poll(fds, MAX_CLIENTS + 1, 100); 

        if(ready < 0) {
            // Nếu bị ngắt bởi tín hiệu hệ thống thì thử lại
            if(errno == EINTR) continue;
            perror("POLL FAIL");
            break;
        }


        // --- GAME LOOP TIMER UPDATE ---
        // Enable for Mode 1 & 2 (multiplayer)
        for (int r = 0; r < MAX_ROOMS; r++) {
             Room *room = room_get_by_id(r);
             if (room) {
                 // Check if game just finished → broadcast end (ONCE)
                 if (room->status == ROOM_FINISHED && room->game_mode != MODE_CLASSIC && !room->end_broadcasted) {
                     broadcast_end_game(r, db);
                     room->end_broadcasted = 1; // Mark as sent
                 }
                 // Regular timer update for PLAYING rooms
                 else if (room->status == ROOM_PLAYING && room->game_mode != MODE_CLASSIC) {
                     int status = room_update_timer(r);
                     if (status == 1) {
                         broadcast_question(r);
                     } else if (status == 2) {
                         broadcast_end_game(r, db);
                         room->end_broadcasted = 1; // Mark as sent
                     }
                 }
             }
        }
        // ------------------------------

        // Kiểm tra socket server & kiểm tra cờ POLLIN được bật
        if (fds[0].revents & POLLIN) {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0){
                perror("ACCEPT FAIL");
            }else{
                printf("New connection, socket fd: %d\n", new_socket);

                // Tìm slot trống nhét client vào
                int found_slot = 0;
                for (i = 1; i < MAX_CLIENTS + 1; i++){
                    if (fds[i].fd == -1){
                        fds[i].fd = new_socket;
                        fds[i].events = POLLIN; // Theo dõi client này
                        found_slot = 1;
                        break;
                    }
                }
                
                // Nếu hết slot trống
                if (!found_slot){
                    printf("Server full\n");
                    buffer[0] = MSG_SERVER_FULL;
                    if(send_with_delimiter(new_socket, buffer, 1) < 0) {
                        perror("SEND SERVER FULL FAIL");
                    }
                    close(new_socket);
                }
            }
        }

        // Kiểm tra các socket client 
        for (i = 1; i < MAX_CLIENTS + 1; i++){
            // Chỉ kiểm tra slot có client
            if(fds[i].fd != -1 && (fds[i].revents & POLLIN)){
                int sd = fds[i].fd;
                int valread = recv(sd, buffer, BUFFER_SIZE - 1, 0);

                // Read trả về 0 -> Client ngắt kết nối
                if(valread == 0){
                    printf("Client %d (fd=%d) disconnected.\n", i, sd);

                    sessions[i].is_logged_in = 0;
                    strcpy(sessions[i].username, "");
                    sessions[i].socket_fd = -1;
                    sessions[i].user_id = -1;

                    close(sd);
                    fds[i].fd = -1;

                // Read > 0 -> Client gửi tín hiệu
                } else if (valread > 0) {
                    buffer[valread] = '\0';
                    unsigned char opcode = buffer[0];
                    char *payload = buffer + 1;

                    // Just print for debug
                    if (opcode == MSG_GET_ROOMS || opcode == MSG_GET_LEADERBOARD || opcode == MSG_GET_ROOM_DETAIL) {
                         // Don't print payload for these to avoid confusion
                         // printf("Client %d sent OpCode: %02x (No Payload)\n", sd, opcode);
                    } else {
                         printf("Client %d sent OpCode: %02x, Payload: %s\n", sd, opcode, payload);
                    }

                    // MOST IMPORTANT 
                    switch (opcode) {
                        case MSG_LOGIN: {
                            char username[50];
                            sscanf(payload, "%s", username);
                            if (handle_login(db, sd, &sessions[i], payload)) {
                                log_history("User '%s' logged in", username);
                            } else {
                                log_history("User '%s' login failed", username);
                            }
                            break;
                        }
                        case MSG_REGISTER: {
                            char username[50];
                            sscanf(payload, "%s", username);
                            if (handle_register(db, sd, payload)) {
                                log_history("User '%s' registered", username);
                            } else {
                                log_history("User '%s' registration failed", username);
                            }
                            break;
                        }
                        case MSG_GET_ONLINE_USERS:
                            handle_get_online_users(sd, fds);
                            break;
                        case MSG_LOGOUT:
                            // Nếu đang trong phòng thì rời phòng
                            room_leave(sessions[i].user_id);
                            
                            log_history("User '%s' logged out", sessions[i].username);
                            sessions[i].is_logged_in = 0;
                            strcpy(sessions[i].username, "");
                            sessions[i].socket_fd = -1;
                            sessions[i].user_id = -1;
                            break;

                        case MSG_GET_LEADERBOARD: {
                            char board[BUFFER_SIZE];
                            get_leaderboard(db, board);
                            char resp[BUFFER_SIZE];
                            resp[0] = MSG_LEADERBOARD_LIST; // 0x46
                            strcpy(resp + 1, board);
                            send_with_delimiter(sd, resp, 1 + strlen(board));
                            // printf("Sent leaderboard to client %d\n", sd);
                            // printf("Sent leaderboard to client %d\n", sd);
                            break;
                        }

                        case MSG_GET_IDLE_USERS:
                            handle_get_idle_users(sd, fds);
                            break;

                        case MSG_INVITE_FRIEND:
                            // Payload is TargetUsername
                            handle_invite_friend(sd, sessions[i].user_id, sessions[i].username, payload, fds);
                            break;

                        case MSG_GET_ROOMS: {
                            char room_list[BUFFER_SIZE];
                            room_get_list_string(room_list);
                            
                            char resp[BUFFER_SIZE];
                            resp[0] = MSG_ROOM_LIST; // 0x28
                            strcpy(resp + 1, room_list);
                            send_with_delimiter(sd, resp, 1 + strlen(room_list));
                            // printf("Sent room list to client %d\n", sd);
                            break;
                        }

                        case MSG_GET_ROOM_DETAIL: {
                            int r_id = atoi(payload);
                            char details[BUFFER_SIZE];
                            room_get_detail_string(r_id, details);
                            
                            char resp[BUFFER_SIZE];
                            resp[0] = MSG_ROOM_DETAIL; // 0x2A
                            strcpy(resp + 1, details);
                            send_with_delimiter(sd, resp, 1 + strlen(details));
                            // printf("Sent details for room %d to client %d\n", r_id, sd); 
                            break;
                        }

                        // --- ROOM HANDLERS ---
                        case MSG_ROOM_CREATE: {
                            int r_id = room_create(sessions[i].user_id, sessions[i].username, sd, payload);
                            char resp[3];
                            if (r_id >= 0) {
                                resp[0] = MSG_ROOM_CREATE;
                                resp[1] = 1; // Success
                                resp[2] = r_id; // Room ID
                                printf("User %s created room %d\n", sessions[i].username, r_id);
                                log_history("User '%s' created room %d", sessions[i].username, r_id);
                            } else {
                                resp[0] = MSG_ROOM_CREATE;
                                resp[1] = 0; // Failure
                                resp[2] = 0;
                                printf("Create room failed for %s. Code: %d\n", sessions[i].username, r_id);
                                log_history("User '%s' create room failed", sessions[i].username);
                            }
                            send_with_delimiter(sd, resp, 3);
                            break;
                        }

                        case MSG_ROOM_JOIN: {
                            int r_id = atoi(payload);
                            int res = room_join(r_id, sessions[i].user_id, sessions[i].username, sd);
                            char resp[2];
                            if (res == 1) {
                                resp[0] = MSG_ROOM_JOIN;
                                resp[1] = 1; 
                                printf("User %s joined room %d\n", sessions[i].username, r_id);
                                log_history("User '%s' joined room %d", sessions[i].username, r_id);
                            } else {
                                resp[0] = MSG_ROOM_JOIN;
                                resp[1] = 0; 
                                printf("Join room failed. Code: %d\n", res);
                                log_history("User '%s' join room %d failed", sessions[i].username, r_id);
                            }
                            send_with_delimiter(sd, resp, 2);
                            break;
                        }

                        case MSG_LEAVE_ROOM: {
                           room_leave(sessions[i].user_id);
                           printf("User %s left room request.\n", sessions[i].username);
                           log_history("User '%s' left room", sessions[i].username);
                           // Gửi lại confirm? Tuỳ protocol, tạm thời ko cần
                           break;
                        }
                        
                        case MSG_GAME_START: {
                            // Find user's room
                            Room *r = room_get_by_user(sessions[i].user_id);
                            if (r) {
                                int res = room_start_game(r->id, sessions[i].user_id);
                                if (res == 1) {
                                    // Start Success -> Broadcast First Question Immediate
                                    printf("User %s started game in room %d\n", sessions[i].username, r->id);
                                    log_history("User '%s' started game in room %d", sessions[i].username, r->id);
                                    broadcast_question(r->id);
                                } else {
                                    printf("Start game failed. Error: %d\n", res);
                                    log_history("User '%s' start game in room %d failed", sessions[i].username, r->id);
                                }
                            }
                            break;
                        }

                        case MSG_ANSWER: {
                            // Payload: "ANSWER_CHAR" (e.g., "A")
                            char result_msg[256];
                            int answer_result = room_handle_answer(sessions[i].user_id, payload, result_msg);
                            
                            // Gửi lải kết quả: MSG_ANSWER_RESULT (0x23) + MSG string
                            char resp[BUFFER_SIZE]; // Use BUFFER_SIZE for consistency
                            resp[0] = MSG_ANSWER_RESULT;
                            
                            // Add elimination flag: "message|eliminated:0/1"
                            Room *r = room_get_by_user(sessions[i].user_id); // Get room here to check elimination status
                            int is_eliminated = 0;
                            if (r) {
                                for (int j = 0; j < r->player_count; j++) {
                                    if (r->members[j].user_id == sessions[i].user_id) {
                                        is_eliminated = r->members[j].is_eliminated;
                                        break;
                                    }
                                }
                            }
                            
                            sprintf(resp + 1, "%s|eliminated:%d", result_msg, is_eliminated);
                            send_with_delimiter(sd, resp, 1 + strlen(resp + 1));
                            
                            // Log debug
                            printf("User %s answered %s. Result: %s\n", sessions[i].username, payload, result_msg);
                            
                            // If answer is correct (return 1), move to next question immediately
                            if (answer_result == 1) {
                                Room *r = room_get_by_user(sessions[i].user_id);
                                if (r && r->status == ROOM_PLAYING) {
                                    r->current_question_idx++;
                                    
                                    // Check if game finished (all 15 questions answered)
                                    if (r->current_question_idx >= 15) {
                                        // Game Over - Won!
                                        printf("[GAME] Room %d finished! User won!\n", r->id);
                                        broadcast_end_game(r->id, db);
                                        r->status = ROOM_FINISHED;
                                    } else {
                                        // Broadcast next question immediately
                                        r->question_start_time = time(NULL);
                                        broadcast_question(r->id);
                                    }
                                }
                            }
                            // If answer is wrong (return 2), player is eliminated - game ends
                            else if (answer_result == 2) {
                                Room *r = room_get_by_user(sessions[i].user_id);
                                if (r) {
                                    printf("[GAME] Room %d - Player eliminated\n", r->id);
                                    broadcast_end_game(r->id, db);
                                    r->status = ROOM_FINISHED;
                                }
                            }
                            
                            break;
                        }
                        
                        case MSG_USE_HELP: {
                            // Payload: "type" (1 byte ascii '1'..'4')
                            int type = atoi(payload);
                            Room* r = room_get_by_user(sessions[i].user_id);
                            char resp[BUFFER_SIZE];
                            resp[0] = MSG_HELP_RESULT; // 0x2D
                            
                            if (r) {
                                char result_msg[256];
                                room_use_lifeline(r->id, sessions[i].user_id, type, result_msg);
                                strcpy(resp + 1, result_msg);
                            } else {
                                strcpy(resp + 1, "Ban khong o trong phong nao!");
                            }
                            send_with_delimiter(sd, resp, 1 + strlen(resp + 1));
                            break;
                        }
                        
                        case MSG_WALK_AWAY: {
                            char result_msg[256];
                            int res = room_walk_away(sessions[i].user_id, result_msg);
                            
                            if (res) {
                                char resp[300];
                                resp[0] = MSG_GAME_END; // Tạm dùng GAME END báo cho rieng user? Hoặc ANSWER_RESULT
                                strcpy(resp + 1, result_msg);
                                send_with_delimiter(sd, resp, 1 + strlen(result_msg));
                            }
                            break;
                        }
                    }
                } else {
                    if (errno != EINTR) {
                        perror("READ FAIL");
                        fds[i].fd = -1;
                        close(sd);
                        
                    }
                }
            }
        }
    }

    // Dọn dẹp trước khi thoát
    for (i = 0; i < MAX_CLIENTS + 1; i++){
        if(fds[i].fd > 0) close(fds[i].fd);
    }

    db_close(db);
    return 0;
}
