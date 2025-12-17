#ifndef ROOM_H
#define ROOM_H


#include "game.h" 

#define MAX_ROOMS 20
#define MAX_PLAYERS_PER_ROOM 4

// Trạng thái phòng
typedef enum {
    ROOM_WAITING = 0,
    ROOM_PLAYING = 1,
    ROOM_FINISHED = 2
} RoomStatus;

// Cấu trúc người chơi trong phòng
typedef struct {
    int user_id;       // ID database
    int socket_fd;     // Socket kết nối
    char username[50];
    int score;
    int is_host;       // 1: Chủ phòng
    int is_eliminated; // 1: Đã bị loại

    // --- TRẠNG THÁI TRỢ GIÚP ---
    int help_5050_used;      // 1. 50:50
    int help_audience_used;  // 2. Khán giả
    int help_phone_used;     // 3. Gọi điện thoại
    int help_expert_used;    // 4. Tư vấn tại chỗ
} RoomMember;

// Cấu trúc phòng
typedef struct {
    int id;
    char name[50];
    RoomStatus status;
    int player_count;
    RoomMember members[MAX_PLAYERS_PER_ROOM];
    
    // Logic Game bên trong phòng
    int question_ids[15];       // Bộ 15 câu hỏi random cho ván này
    int current_question_idx;   // Index câu hiện tại (0-14)
    time_t question_start_time; // Thời điểm bắt đầu câu hỏi hiện tại
    

} Room;

// Khởi tạo hệ thống phòng
void room_system_init(void *db_conn);

// API quản lý phòng
int room_create(int user_id, char *username, int socket_fd, char *room_name);
int room_join(int room_id, int user_id, char *username, int socket_fd);
int room_leave(int user_id); // Tìm user đang ở phòng nào và rời đi
int room_start_game(int room_id, int user_id); // Chỉ host mới start được

// API hỗ trợ Game Loop (Timer)
int room_update_timer(int room_id);

// Lấy danh sách phòng (Format string)
void room_get_list_string(char *buffer);

// FORMAT: "host_flag:username:score,..."
void room_get_detail_string(int room_id, char *buffer);

int room_handle_answer(int user_id, char *answer, char *result_msg);
int room_walk_away(int user_id, char *result_msg);

// Lấy phòng theo ID
Room* room_get_by_id(int room_id);
// Lấy phòng theo User ID
Room* room_get_by_user(int user_id);
// Sử dụng trợ giúp trong phòng
int room_use_lifeline(int room_id, int user_id, int lifeline_type, char *result_msg);

#endif