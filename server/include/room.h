#ifndef ROOM_H
#define ROOM_H


#include "game.h" 

#define MAX_ROOMS 20
#define MAX_PLAYERS_PER_ROOM 4

// Game Mode Constants
#define MODE_CLASSIC 0      // Cổ Điển (đơn, mốc an toàn)
#define MODE_COOP 1         // Hợp Tác (multi, đồng đội, chung trợ giúp)
#define MODE_SCORE_ATTACK 2 // Tính Điểm (multi, tất cả trả lời)

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
    int has_answered;  // 1: Đã trả lời câu hiện tại (Mode 2)

    // --- TRẠNG THÁI TRỢ GIÚP (Cho Mode Private/Classic) ---
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
    Question questions[15];       // Bộ 15 câu hỏi random cho ván này (lưu full struct)
    int current_question_idx;   // Index câu hiện tại (0-14)
    time_t question_start_time; // Thời điểm bắt đầu câu hỏi hiện tại
    
    char game_log[4096]; // Log diễn biến: "UserID:QuestionID:Answer,"
    int game_mode; // 0: Classic, 1: Coop, 2: Speed Attack
    int game_id; // Database game_history ID (for saving stats)
    int end_broadcasted; // Flag: 1 if game end already broadcasted

    // --- TRẠNG THÁI TRỢ GIÚP DÙNG CHUNG (Cho Mode Coop) ---
    int shared_help_5050_used;
    int shared_help_audience_used;
    int shared_help_phone_used;
    int shared_help_expert_used;
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

// Mode-specific answer handlers
int room_handle_answer_practice(int user_id, char *answer, char *result_msg);
int room_handle_answer_coop(int user_id, char *answer, char *result_msg);
int room_handle_answer_speedattack(int user_id, char *answer, char *result_msg);

// Dispatcher (calls appropriate mode handler)
int room_handle_answer(int user_id, char *answer, char *result_msg);
int room_walk_away(int user_id, char *result_msg);

// Lấy phòng theo ID
Room* room_get_by_id(int room_id);
// Lấy phòng theo User ID
Room* room_get_by_user(int user_id);
// Sử dụng trợ giúp trong phòng
int room_use_lifeline(int room_id, int user_id, int lifeline_type, char *result_msg);

// Kiểm tra xem tất cả người chơi đã bị loại chưa
int room_all_eliminated(int room_id);

// Xóa người chơi khỏi phòng (Mode 1 elimination)
void room_remove_player(int room_id, int user_id);

// Kiểm tra xem tất cả người chơi đã trả lời câu hiện tại (Mode 2)
int all_players_answered(int room_id);

// Reset trạng thái trả lời cho câu mới (Mode 2)
void reset_answer_flags(int room_id);

#endif