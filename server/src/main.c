#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <poll.h>
#include <errno.h>

#include "database.h"
#include "protocol.h"
#include "room.h"
#include "game.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100


User users[MAX_CLIENTS];
Session sessions[MAX_CLIENTS+1];
int user_count = 0;

void init_session() {
    for (int i = 0; i < MAX_CLIENTS + 1; i++){
        sessions[i].socket_fd = -1;
        sessions[i].is_logged_in = 0;
        sessions[i].user_id = -1;
        strcpy(sessions[i].username, "");
    }
}

int is_user_online(char *username) {
    for (int i = 0; i < MAX_CLIENTS + 1; i++){
        if(sessions[i].is_logged_in && strcmp(sessions[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
}

void handle_get_online_users(int client_fd, struct pollfd* fds) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    buffer[0] = MSG_ONLINE_USERS_RESULT;
    char *list_ptr = buffer + 1; 
    int count = 0;
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {

        if (fds[i].fd != -1 && fds[i].fd != client_fd && sessions[i].is_logged_in == 1) {
            strcat(list_ptr, sessions[i].username);
            strcat(list_ptr, ","); 
            count++;
        }
    }
    int len = strlen(list_ptr);
    if (len > 0 && list_ptr[len - 1] == ',') {
        list_ptr[len - 1] = '\0';
    }
    printf("Sending online list to %d: %s\n", client_fd, list_ptr);

    // Gửi về client: [0x08] + "user1,user2,user3"
    send(client_fd, buffer, 1 + strlen(list_ptr), 0);
}

int find_user(char *username){
    for (int i = 0; i < user_count; i ++){
        if(strcmp(users[i].username, username) == 0) return i;
    }
    return -1;
}

// DONE
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
        printf("Register failed (User exists): %s\n", username);
    }
    send(client_fd, response, 1, 0);
}

// DONE
void handle_login(sqlite3* db,int client_fd, int session_index, char *payload) {
    char username[50], password[50];
    if (sscanf(payload, "%s %s", username, password) < 2) return;
    char response[1];

    if(is_user_online(username)) {
        response[0] = MSG_ALREADY_LOGIN;
        send(client_fd, response, 1, 0);
        return;
    }

    int user_id = verify_user(db, username, password);
    if (user_id > 0) {
        sessions[session_index].is_logged_in = 1;
        sessions[session_index].socket_fd = client_fd;
        sessions[session_index].user_id = user_id;
        strcpy(sessions[session_index].username, username);

        response[0] = MSG_LOGIN_SUCCESS;
        printf("User '%s' logged in (ID: %d) on session slot %d\n", username, user_id, session_index);
    } else {
        response[0] = MSG_LOGIN_FAILED;
        printf("User '%s' login failed\n", username);
    }
    send(client_fd, response, 1, 0);
    
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
        // Block đến khi có 1 sự kiện xảy ra
        int ready = poll(fds, MAX_CLIENTS + 1, -1); // -1: Chờ vô hạn

        if(ready < 0) {
            // Nếu bị ngắt bởi tín hiệu hệ thống thì thử lại
            if(errno == EINTR) continue;
            perror("POLL FAIL");
            break;
        }

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
                    if(send(new_socket, buffer, 1, 0) <= 0) {
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

                    printf("Client %d sent OpCode: %02x, Payload: %s\n", sd, opcode, payload);

                    // MOST IMPORTANT 
                    switch (opcode) {
                        case MSG_LOGIN:
                            handle_login(db, sd, i, payload);
                            break;
                        case MSG_REGISTER:
                            handle_register(db, sd, payload);
                            break;
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

                        case MSG_GET_ROOMS: {
                            char room_list[BUFFER_SIZE];
                            room_get_list_string(room_list);
                            
                            char resp[BUFFER_SIZE];
                            resp[0] = MSG_ROOM_LIST; // 0x28
                            strcpy(resp + 1, room_list);
                            send(sd, resp, 1 + strlen(room_list), 0);
                            printf("Sent room list to client %d\n", sd);
                            break;
                        }

                        // --- ROOM HANDLERS ---
                        case MSG_ROOM_CREATE: {
                            int r_id = room_create(sessions[i].user_id, sessions[i].username, sd, payload);
                            char resp[2];
                            if (r_id >= 0) {
                                resp[0] = MSG_ROOM_CREATE;
                                resp[1] = 1; // Success
                                printf("User %s created room %d\n", sessions[i].username, r_id);
                            } else {
                                resp[0] = MSG_ROOM_CREATE;
                                resp[1] = 0; // Failure
                                printf("Create room failed for %s. Code: %d\n", sessions[i].username, r_id);
                            }
                            send(sd, resp, 2, 0);
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
                            } else {
                                resp[0] = MSG_ROOM_JOIN;
                                resp[1] = 0; 
                                printf("Join room failed. Code: %d\n", res);
                            }
                            send(sd, resp, 2, 0);
                            break;
                        }

                        case MSG_LEAVE_ROOM: {
                           room_leave(sessions[i].user_id);
                           printf("User %s left room request.\n", sessions[i].username);
                           // Gửi lại confirm? Tuỳ protocol, tạm thời ko cần
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
