#ifndef PROTOCOL_H
#define PROTOCOL_H

// --- GROUP 0x00: AUTHENTICATION ---
#define MSG_LOGIN            0x01
#define MSG_REGISTER         0x02
#define MSG_LOGIN_SUCCESS    0x03
#define MSG_LOGIN_FAILED     0x04
#define MSG_ALREADY_LOGIN    0x05
#define MSG_REGISTER_SUCCESS 0x06
#define MSG_REGISTER_FAILED  0x07
#define MSG_SERVER_FULL      0x08
#define MSG_LOGOUT           0x09

// --- GROUP 0x10: ROOM MANAGEMENT ---
#define MSG_ROOM_CREATE      0x10
#define MSG_ROOM_JOIN        0x11
#define MSG_ROOM_LEAVE       0x12 // Request to leave
#define MSG_ROOM_LIST        0x13 // Response list
#define MSG_GET_ROOMS        0x14 // Request list
#define MSG_ROOM_DETAIL      0x15 // Response details
#define MSG_GET_ROOM_DETAIL  0x16 // Request details
#define MSG_ROOM_UPDATE      0x17 // Broadcast update

// --- GROUP 0x20: GAMEPLAY ---
#define MSG_GAME_START       0x20
#define MSG_QUESTION         0x21
#define MSG_ANSWER           0x22
#define MSG_ANSWER_RESULT    0x23
#define MSG_GAME_END         0x24
#define MSG_SCORE_UPDATE     0x25
#define MSG_WALK_AWAY        0x26
#define MSG_USE_HELP         0x27
#define MSG_HELP_RESULT      0x28

// --- GROUP 0x40: SOCIAL & INFO ---
#define MSG_GET_ONLINE_USERS    0x40
#define MSG_ONLINE_USERS_RESULT 0x41
#define MSG_GET_IDLE_USERS      0x42
#define MSG_IDLE_USERS_LIST     0x43
#define MSG_INVITE_RECEIVED     0x44
#define MSG_GET_LEADERBOARD     0x45
#define MSG_LEADERBOARD_LIST    0x46
#define MSG_INVITE_FRIEND       0x47


// --- GROUP 0x50: ADMIN ---
#define MSG_GET_ALL_USERS       0x50
#define MSG_ALL_USERS_RESULT    0x51
#define MSG_DELETE_USER         0x52
#define MSG_DELETE_USER_RESULT  0x53
#define MSG_GET_USER_DETAIL     0x54
#define MSG_USER_DETAIL_RESULT  0x55

// --- GROUP 0x60: USER DATA ---
#define MSG_REFRESH_USER_INFO   0x60
#define MSG_USER_INFO_UPDATE    0x61

// --- GROUP 0x70: HISTORY ---
#define MSG_GET_GAME_HISTORY        0x70
#define MSG_GAME_HISTORY_RESPONSE   0x71
#define MSG_GET_QUESTIONS_BY_IDS    0x72
#define MSG_QUESTIONS_RESPONSE      0x73

// Database Structs
typedef struct {
    char username[50];
    char password[50];
} User;

typedef struct {
    int socket_fd;
    int user_id;
    char username[50];
    int is_logged_in;
    int role; // 0: Admin, 1: User
} Session;

#endif