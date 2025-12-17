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

#define MSG_LOGIN 0x01
#define MSG_REGISTER 0x02
#define MSG_LOGIN_SUCCESS 0x03
#define MSG_LOGIN_FAILED 0x04
#define MSG_REGISTER_SUCCESS 0x05
#define MSG_REGISTER_FAILED 0x06
#define MSG_SERVER_FULL 0x07

#define MSG_GET_ONLINE_USERS 0x08
#define MSG_ONLINE_USERS_RESULT 0x09

#define MSG_ROOM_CREATE 0x10
#define MSG_ROOM_JOIN 0x11
#define MSG_ROOM_UPDATE 0x12
#define MSG_ROOM_INVITE 0x13

#define MSG_GAME_START 0x20
#define MSG_QUESTION 0x21
#define MSG_ANSWER 0x22
#define MSG_ANSWER_RESULT 0x23
#define MSG_ELIMINATE 0x24
#define MSG_SCORE_UPDATE 0x25
#define MSG_GAME_END 0x26

#define MSG_LOGOUT 0x30
#define MSG_LEAVE_ROOM 0x31

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

typedef struct {
    char username[50];
    char password[50];
} User;

typedef struct {
    int socket_fd;
    char username[50];
    int is_logged_in;
} Session;

User users[MAX_CLIENTS];
int user_count = 0;

Session sessions[MAX_CLIENTS+1];
void init_session() {
    for (int i = 0; i < MAX_CLIENTS + 1; i++){
        sessions[i].socket_fd = -1;
        sessions[i].is_logged_in = 0;
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



void load_users_from_db() {
    FILE *f = fopen("users.txt", "r");
    if (f == NULL ) return;

    user_count = 0;
    // DOC TRONG File
    while(fscanf(f, "%s %s", users[user_count].username, users[user_count].password) != EOF) {
        user_count += 1;
        if(user_count >= MAX_CLIENTS) break;
    }

    fclose(f);
    printf("Loaded %d users from database.\n", user_count);

}

void save_user_to_db(char *username, char *password) {
    FILE *f = fopen("users.txt", "a");
    if (f == NULL) {
        perror("CANNOT OPEN FILE");
        return;
    }
    // Luu vao trong File
    fprintf(f, "%s %s", username, password);
    fclose(f);
}

int find_user(char *username){
    for (int i = 0; i < user_count; i ++){
        if(strcmp(users[i].username, username) == 0) return i;
    }
    return -1;
}

void handle_register(int client_fd, char *payload) {
    char username[50], password[50];
    if (sscanf(payload, "%s %s", username, password) < 2) return;

    char response[1];
    if (find_user(username) != -1) {
        response[0] = MSG_REGISTER_FAILED;
    } else {
        strcpy(users[user_count].username, username);
        strcpy(users[user_count].password, password);
        user_count++;

        save_user_to_db(username, password);

        response[0] = MSG_REGISTER_SUCCESS;
        printf("User registered: %s\n", username);
    }
    send(client_fd, response, 1, 0);
}

void handle_login(int client_fd, int session_index, char *payload) {
    char username[50], password[50];
    if (sscanf(payload, "%s %s", username, password) < 2) return;
    char response[1];

    int user_id = find_user(username);

    if(user_id == -1 ||strcmp(users[user_id].password, password) != 0){
        response[0] = MSG_LOGIN_FAILED;
        send(client_fd, response, 1, 0);
    }
    if(is_user_online(username)) {
        response[0] = MSG_LOGIN_FAILED;
        send(client_fd, response, 1, 0);
    }

    sessions[session_index].is_logged_in = 1;
    strcpy(sessions[session_index].username, username);
    sessions[session_index].socket_fd = client_fd;

    response[0] = MSG_LOGIN_SUCCESS;
    printf("Session Created: User '%s' on FD %d\n", username, client_fd);
    send(client_fd, response, 1, 0);
}

int main(){
    load_users_from_db();

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
                int valread;

                // Read trả về 0 -> Client ngắt kết nối
                if((valread = recv(sd, buffer, BUFFER_SIZE - 1, 0)) == 0){
                    printf("Client %d (fd=%d) disconnected.\n", i, sd);

                    sessions[i].is_logged_in = 0;
                    strcpy(sessions[i].username, "");
                    sessions[i].socket_fd = -1;

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
                            handle_login(sd, i, payload);
                            break;
                        case MSG_REGISTER:
                            handle_register(sd, payload);
                            break;
                        case MSG_GET_ONLINE_USERS:
                            handle_get_online_users(sd, fds);
                            break;


                        
                    }




                // Read = -1 Lỗi khi đọc 
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
    return 0;
}
