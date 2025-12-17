#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MSG_LOGIN            0x01
#define MSG_REGISTER         0x02
#define MSG_LOGIN_SUCCESS    0x03
#define MSG_LOGIN_FAILED     0x04
#define MSG_ALREADY_LOGIN    0x05
#define MSG_REGISTER_SUCCESS 0x06
#define MSG_REGISTER_FAILED  0x07
#define MSG_SERVER_FULL      0x08

#define MSG_ROOM_CREATE 0x10
#define MSG_ROOM_JOIN 0x11
#define MSG_ROOM_UPDATE 0x12
#define MSG_ROOM_INVITE 0x13
#define MSG_GET_ROOMS 0x27
#define MSG_ROOM_LIST 0x28

#define MSG_GAME_START 0x20
#define MSG_QUESTION 0x21
#define MSG_ANSWER 0x22
#define MSG_ANSWER_RESULT 0x23
#define MSG_ELIMINATE 0x24
#define MSG_SCORE_UPDATE 0x25
#define MSG_GAME_END 0x26
#define MSG_GET_ROOMS 0x27
#define MSG_ROOM_LIST 0x28
#define MSG_GET_ROOM_DETAIL 0x29
#define MSG_ROOM_DETAIL 0x2A

#define MSG_LOGOUT 0x30
#define MSG_LOGOUT 0x30
#define MSG_LEAVE_ROOM 0x31
#define MSG_WALK_AWAY 0x2B
#define MSG_USE_HELP 0x2C
#define MSG_HELP_RESULT 0x2D


#define MSG_GET_ONLINE_USERS 0x40
#define MSG_ONLINE_USERS_RESULT 0x41
#define MSG_GET_LEADERBOARD 0x45
#define MSG_LEADERBOARD_LIST 0x46

// Cấu trúc ngừoi dùng trong gói tin
typedef struct {
    char username[50];
    char password[50];
} User;

// Cấu trúc Session
typedef struct {
    int socket_fd;
    int user_id;
    char username[50];
    int is_logged_in;
} Session;


#endif