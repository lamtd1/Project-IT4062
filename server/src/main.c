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
#define PORT 8080


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

// Trả về các user online, trừ chính mình và admin
void handle_get_online_users(int client_fd, struct pollfd* fds) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    buffer[0] = MSG_ONLINE_USERS_RESULT;
    char *list_ptr = buffer + 1; 
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {
        // Bỏ chính mình, admin (role=0)
        if (fds[i].fd != -1 && fds[i].fd != client_fd && sessions[i].is_logged_in == 1) {
            if (sessions[i].role == 0) continue;
            
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
// Lấy user online, chưa ở trong phòng nào
void handle_get_idle_users(int client_fd, struct pollfd* fds) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    buffer[0] = MSG_IDLE_USERS_LIST; // 0x43
    char *list_ptr = buffer + 1; 
    // Lọc user online, ngoài bản thân, và KHÔNG PHẢI ADMIN
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {
        if (fds[i].fd != -1 && fds[i].fd != client_fd && sessions[i].is_logged_in == 1) {
            // Check not admin (role != 0)
            if (sessions[i].role == 0) continue;

            // và chưa trong phòng nào
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
    printf("Sending idle list to %d: %s\n", client_fd, list_ptr);
    send_with_delimiter(client_fd, buffer, 1 + strlen(list_ptr));
}

// Mời vào phòng
void handle_invite_friend(int sender_fd, int sender_id, char* sender_name, char* target_name, struct pollfd* fds) {
    // 1. Tìm phòng người gửi
    Room *r = room_get_by_user(sender_id);
    if (!r) return; // Sender not in room

    // 2. Tìm socket của người được mời
    int target_fd = -1;
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {
        if (sessions[i].is_logged_in && strcmp(sessions[i].username, target_name) == 0) {
            target_fd = fds[i].fd;
            break;
        }
    }
    if (target_fd != -1) {
        // 3. Gửi lời mời: [0x44] "SenderName:RoomID"
        char buffer[BUFFER_SIZE];
        buffer[0] = MSG_INVITE_RECEIVED; // 0x44
        
        // Format payload: SenderName:RoomID
        sprintf(buffer + 1, "%s:%d", sender_name, r->id);
        
        send_with_delimiter(target_fd, buffer, 1 + strlen(buffer + 1));
        printf("User %s invited %s to room %d\n", sender_name, target_name, r->id);
    } else {
        printf("Invite failed: User %s not found or offline\n", target_name);
    }
}

int main(){

    // Điều hướng stdout vào file để log
    if (freopen("../../logs/server_history.txt", "a", stdout) == NULL) {
        perror("Không thể chuyển hướng stdout");
    }
    if (freopen("../../logs/server_history.txt", "a", stderr) == NULL) {
        perror("Không thể chuyển hướng stderr");
    }
    // Thiết lập line buffering để log được ghi theo thời gian thực
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    // Khởi tạo database
    sqlite3 *db = db_init("../../database/database.db");
    if(!db) {
        fprintf(stderr, "Failed to connect to database\n");
        return -1;
    }

    // Khởi tạo session
    init_session();
    // Khởi tạo room
    room_system_init(db);
    // Load tất cả câu hỏi vào cache (1 lần duy nhất khi server khởi động)
    game_init(db);


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
        for (int r = 0; r < MAX_ROOMS; r++) {
             // room_update_timer trả về: 1 (Next Q), 2 (End)
             int status = room_update_timer(r);
             if (status == 1) {
                 broadcast_question(r);
             } else if (status == 2) {
                 broadcast_end_game(r, db);
             }
        }
        // ------------------------------

        // Kiểm tra socket server & kiểm tra cờ POLLIN được bật
        if (fds[0].revents & POLLIN) {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0){
                perror("ACCEPT FAIL");
            }else{
                printf("New connection, socket fd: %d, IP: %s, Port: %d\n", 
                       new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

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

                    // Tự động rời phòng nếu đang trong phòng
                    if (sessions[i].user_id > 0) {
                        room_leave(sessions[i].user_id);
                    }

                    sessions[i].is_logged_in = 0;
                    strcpy(sessions[i].username, "");
                    sessions[i].socket_fd = -1;
                    sessions[i].user_id = -1;

                    close(sd);
                    fds[i].fd = -1;
                } else if (valread > 0) {
                    buffer[valread] = '\0';
                    unsigned char opcode = buffer[0];
                    char *payload = buffer + 1;

                    // debug
                    if (opcode == MSG_GET_ROOMS || opcode == MSG_GET_LEADERBOARD || opcode == MSG_GET_ROOM_DETAIL || opcode == MSG_GET_ALL_USERS || opcode == MSG_REFRESH_USER_INFO) {
                         printf("Client %d sent OpCode: %02x (No Payload)\n", sd, opcode);
                    } else {
                         printf("Client %d sent OpCode: %02x, Payload: %s\n", sd, opcode, payload);
                    }

                    // MOST IMPORTANT 
                    switch (opcode) {
                        case MSG_LOGIN: {
                            handle_login(db, sd, &sessions[i], payload);
                            break;
                        }
                        case MSG_REGISTER: {
                            handle_register(db, sd, payload);
                            break;
                        }
                        case MSG_GET_ONLINE_USERS:
                            handle_get_online_users(sd, fds);
                            break;
                        case MSG_LOGOUT:
                            // Nếu đang trong phòng thì rời phòng
                            room_leave(sessions[i].user_id);
                            
                            sessions[i].is_logged_in = 0;
                            strcpy(sessions[i].username, "");
                            sessions[i].socket_fd = -1;
                            sessions[i].user_id = -1;
                            break;

                        case MSG_GET_LEADERBOARD: {
                            char board[BUFFER_SIZE];
                            get_leaderboard(db, board);
                            char resp[BUFFER_SIZE];
                            resp[0] = MSG_LEADERBOARD_LIST; 
                            strcpy(resp + 1, board);
                            send_with_delimiter(sd, resp, 1 + strlen(board));
                            break;
                        }
                        
                        case MSG_REFRESH_USER_INFO: {
                            // Send updated user info (score) back to client
                            int user_score = get_user_score(db, sessions[i].user_id);
                            char response[64];
                            response[0] = MSG_USER_INFO_UPDATE;
                            sprintf(response + 1, "%d", user_score);
                            send_with_delimiter(sd, response, 1 + strlen(response + 1));
                            printf("[USER_INFO] Sent updated score %d to user %d\n", user_score, sessions[i].user_id);
                            break;
                        }
                        
                        case MSG_GET_GAME_HISTORY: {
                            char history_buffer[8192];
                            printf("[GAME_HISTORY] Querying history for user_id=%d\n", sessions[i].user_id);
                            get_user_game_history(db, sessions[i].user_id, history_buffer, sizeof(history_buffer));
                            
                            printf("[GAME_HISTORY] Query result: %lu bytes\n", strlen(history_buffer));
                            if (strlen(history_buffer) > 0) {
                                printf("[GAME_HISTORY] Sample data: %.200s...\n", history_buffer);
                            } else {
                                printf("[GAME_HISTORY] WARNING: Empty result for user_id=%d\n", sessions[i].user_id);
                            }
                            
                            char response[8300];
                            response[0] = MSG_GAME_HISTORY_RESPONSE;
                            strcpy(response + 1, history_buffer);
                            send_with_delimiter(sd, response, 1 + strlen(response + 1));
                            printf("[GAME_HISTORY] Sent history response (%lu bytes total)\n", 
                                   1 + strlen(history_buffer));
                            break;
                        }
                        
                        case MSG_GET_QUESTIONS_BY_IDS: {
                            const char *ids_str = buffer + 1;
                            char questions_buffer[8192];
                            get_questions_by_ids(db, ids_str, questions_buffer, sizeof(questions_buffer));
                            
                            char response[8300];
                            response[0] = MSG_QUESTIONS_RESPONSE;
                            strcpy(response + 1, questions_buffer);
                            send_with_delimiter(sd, response, 1 + strlen(response + 1));
                            printf("[QUESTIONS] Sent %lu bytes for IDs: %s\n", strlen(questions_buffer), ids_str);
                            break;
                        }

                        case MSG_GET_IDLE_USERS:
                            handle_get_idle_users(sd, fds);
                            break;

                        case MSG_INVITE_FRIEND:
                            // Payload là TargetUsername
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
                            // payload là RoomID:mode
                            int r_id = atoi(payload);
                            char details[BUFFER_SIZE];
                            room_get_detail_string(r_id, details);
                            
                            char resp[BUFFER_SIZE];
                            resp[0] = MSG_ROOM_DETAIL; // 0x2A
                            strcpy(resp + 1, details);
                            send_with_delimiter(sd, resp, 1 + strlen(details));
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
                            } else {
                                resp[0] = MSG_ROOM_CREATE;
                                resp[1] = 0; // Failure
                                resp[2] = 0;
                                printf("Create room failed for %s. Code: %d\n", sessions[i].username, r_id);
                            }
                            printf("[MAIN] DEBUG: Created room. Session Username: '%s'\n", sessions[i].username);
                            send_with_delimiter(sd, resp, 3);
                            break;
                        }

                        case MSG_ROOM_JOIN: {
                            int r_id = atoi(payload);
                            int res = room_join(r_id, sessions[i].user_id, sessions[i].username, sd);
                            printf("[MAIN] DEBUG: User %d joining room. Session Username: '%s'\n", sessions[i].user_id, sessions[i].username);

                            char resp[2];
                            if (res == 1) {
                                resp[0] = MSG_ROOM_JOIN;
                                resp[1] = 1; 
                                printf("User %s joined room %d\n", sessions[i].username, r_id);
                            } else {
                                resp[0] = MSG_ROOM_JOIN;
                                resp[1] = 0; 
                                printf("Join room failed. Code: %d\n", res);
                            }
                            send_with_delimiter(sd, resp, 2);
                            break;
                        }

                        case MSG_ROOM_LEAVE: {
                           room_leave(sessions[i].user_id);
                           printf("User %s left room request.\n", sessions[i].username);
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
                                    
                                    // Broadcast initial scores (all 0) so UI shows player list
                                    broadcast_scores(r->id);
                                    
                                    broadcast_question(r->id);
                                } else {
                                    printf("Start game failed. Error: %d\n", res);
                                }
                            }
                            break;
                        }

                        // Điều hướng câu trả lời theo game mode
                        case MSG_ANSWER: {
                            // Payload: "ANSWER_CHAR" 
                            char result_msg[256];
                            int answer_result = room_handle_answer(sessions[i].user_id, payload, result_msg);
                            
                            // Gửi kết quả: MSG_ANSWER_RESULT + MSG string
                            char resp[300];
                            resp[0] = MSG_ANSWER_RESULT;
                            strcpy(resp + 1, result_msg);
                            send_with_delimiter(sd, resp, 1 + strlen(result_msg));
                            
                            // Log debug
                            printf("User %s answered %s. Result: %s\n", sessions[i].username, payload, result_msg);
                            
                            Room *r = room_get_by_user(sessions[i].user_id);
                            if (!r) break;
                            
                            // Return codes:
                            // 0 = Wrong (Mode 0/2 no elimination)
                            // 1 = Correct (Mode 0/2 wait)
                            // 2 = Wrong + Eliminated (Mode 1)
                            // 3 = Correct + Instant Advance (Mode 1)
                            // 4 = All Answered → Advance (Mode 2)
                            
                            if (answer_result == 1 && r->game_mode == MODE_CLASSIC) {
                                // Mode 0: Advance immediately
                                r->current_question_idx++;
                                if (r->current_question_idx >= 15) {
                                    printf("[GAME] Room %d finished!\n", r->id);
                                    broadcast_end_game(r->id, db);
                                    r->status = ROOM_FINISHED;
                                } else {
                                    r->question_start_time = time(NULL);
                                    broadcast_question(r->id);
                                }
                            }
                            else if (answer_result == 3) {
                                // Mode 1: Instant advance (first correct answer)
                                r->current_question_idx++;
                                reset_answer_flags(r->id); // Reset for next question
                                
                                if (r->current_question_idx >= 15) {
                                    printf("[MODE1] Room %d finished!\n", r->id);
                                    broadcast_end_game(r->id, db);
                                    r->status = ROOM_FINISHED;
                                } else {
                                    r->question_start_time = time(NULL);
                                    broadcast_question(r->id);
                                }
                            }
                            else if (answer_result == 4) {
                                // Mode 2: All answered → Advance
                                r->current_question_idx++;
                                reset_answer_flags(r->id); // Reset for next question
                                
                                if (r->current_question_idx >= 15) {
                                    printf("[MODE2] Room %d finished!\n", r->id);
                                    broadcast_end_game(r->id, db);
                                    r->status = ROOM_FINISHED;
                                } else {
                                    r->question_start_time = time(NULL);
                                    broadcast_question(r->id);
                                }
                            }
                            else if (answer_result == 2) {
                                // Player eliminated (stays in room, marked as is_eliminated)
                                printf("[GAME] Room %d - Player %s eliminated\n", r->id, sessions[i].username);
                                
                                // Check if ALL players eliminated
                                if (r && room_all_eliminated(r->id)) {
                                    printf("[GAME] Room %d - All players eliminated. Game over.\n", r->id);
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
                                strcpy(resp + 1, "Bạn không ở phòng nào!");
                            }
                            send_with_delimiter(sd, resp, 1 + strlen(resp + 1));
                            break;
                        }
                        
                        // ========== ADMIN HANDLERS ==========
                        case MSG_GET_ALL_USERS: {
                            // Only admin can access this
                            int role = get_user_role(db, sessions[i].user_id);
                            if (role != 0) {
                                // Not admin, send empty response
                                char resp[2];
                                resp[0] = MSG_ALL_USERS_RESULT;
                                resp[1] = '\0';
                                send_with_delimiter(sd, resp, 1);
                                break;
                            }
                            
                            char users_data[BUFFER_SIZE];
                            get_all_users_for_admin(db, users_data);
                            
                            // Now append online status for each user
                            // Format: "id:username:total_win:total_score:is_online,..."
                            char final_buffer[BUFFER_SIZE];
                            final_buffer[0] = '\0';
                            
                            // Parse users_data and add online status
                            char *token = strtok(users_data, ",");
                            while (token != NULL) {
                                int uid;
                                char uname[50];
                                int uwins, uscore;
                                sscanf(token, "%d:%[^:]:%d:%d", &uid, uname, &uwins, &uscore);
                                
                                // Check if user is online in sessions
                                int is_online = 0;
                                for (int j = 1; j < MAX_CLIENTS + 1; j++) {
                                    if (sessions[j].is_logged_in && sessions[j].user_id == uid) {
                                        is_online = 1;
                                        break;
                                    }
                                }
                                
                                char line[300];
                                sprintf(line, "%d:%s:%d:%d:%d,", uid, uname, uwins, uscore, is_online);
                                strcat(final_buffer, line);
                                
                                token = strtok(NULL, ",");
                            }
                            
                            // Remove trailing comma
                            int len = strlen(final_buffer);
                            if (len > 0 && final_buffer[len-1] == ',') final_buffer[len-1] = '\0';
                            
                            char resp[BUFFER_SIZE];
                            resp[0] = MSG_ALL_USERS_RESULT;
                            strcpy(resp + 1, final_buffer);
                            send_with_delimiter(sd, resp, 1 + strlen(final_buffer));
                            break;
                        }
                        
                        case MSG_DELETE_USER: {
                            // Only admin can delete
                            int role = get_user_role(db, sessions[i].user_id);
                            if (role != 0) {
                                char resp[2];
                                resp[0] = MSG_DELETE_USER_RESULT;
                                resp[1] = '0'; // Failed
                                send_with_delimiter(sd, resp, 2);
                                break;
                            }
                            
                            int target_user_id = atoi(payload);
                            int success = soft_delete_user(db, target_user_id);
                            
                            // If user is online, kick them out
                            if (success) {
                                for (int j = 1; j < MAX_CLIENTS + 1; j++) {
                                    if (sessions[j].user_id == target_user_id && sessions[j].is_logged_in) {
                                        // Force logout
                                        room_leave(sessions[j].user_id);
                                        sessions[j].is_logged_in = 0;
                                        sessions[j].user_id = -1;
                                        strcpy(sessions[j].username, "");
                                        
                                        // Optionally send disconnect message
                                        char kick_msg[2];
                                        kick_msg[0] = MSG_LOGOUT;
                                        send_with_delimiter(fds[j].fd, kick_msg, 1);
                                        break;
                                    }
                                }
                            }
                            
                            char resp[2];
                            resp[0] = MSG_DELETE_USER_RESULT;
                            resp[1] = success ? '1' : '0';
                            send_with_delimiter(sd, resp, 2);
                            break;
                        }
                        
                        case MSG_GET_USER_DETAIL: {
                            // Only admin can view details
                            int role = get_user_role(db, sessions[i].user_id);
                            if (role != 0) {
                                char resp[2];
                                resp[0] = MSG_USER_DETAIL_RESULT;
                                resp[1] = '\0';
                                send_with_delimiter(sd, resp, 1);
                                break;
                            }
                            
                            int target_user_id = atoi(payload);
                            char user_detail[256];
                            get_user_detail(db, target_user_id, user_detail);
                            
                            char resp[300];
                            resp[0] = MSG_USER_DETAIL_RESULT;
                            strcpy(resp + 1, user_detail);
                            send_with_delimiter(sd, resp, 1 + strlen(user_detail));
                            break;
                        }
                        // walk away là được giữ điểm
                        case MSG_WALK_AWAY: {
                            char result_msg[256];
                            int res = room_walk_away(sessions[i].user_id, result_msg);
                            
                            if (res) {
                                char resp[300];
                                resp[0] = MSG_WALK_AWAY;  // Gửi đúng opcode để client xử lý
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
