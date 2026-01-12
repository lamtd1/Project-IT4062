#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/room.h"
#include "../include/network.h"  // For broadcast_scores()

// --- KHAI BÁO THƯ VIỆN ĐỂ SEND ---
#include <sys/socket.h>
#include <unistd.h>
#include "../include/protocol.h"

// --- KHAI BÁO CÁC HÀM TỪ GAME.C ---

extern void get_5050_options(Question *q, char *out_str);
extern void get_audience_stats(Question *q, char *out_str);
extern void get_phone_friend_response(Question *q, char *out_str);
extern void get_expert_advice(Question *q, char *out_str);
extern int load_room_questions(void *db_conn, Question *questions_array);
extern int calculate_score(Question *q, char *answer, double elapsed_time);
extern int get_prize_for_level(int level);
extern int calculate_safe_reward(int level);
// ----------------------------------

// --- BIẾN TOÀN CỤC ---
static Room rooms[MAX_ROOMS];  // Mảng lưu trữ tất cả các phòng
static void *global_db = NULL; // Lưu connection đến database để dùng trong các hàm room

/**
 * Khởi tạo hệ thống phòng chơi
 * @param db_conn: Con trỏ đến kết nối database
 * 
 * NOTE: Hàm này được gọi 1 lần duy nhất khi server khởi động
 */
void room_system_init(void *db_conn) {
    global_db = db_conn; // Lưu DB connection để dùng sau này
    
    // Khởi tạo tất cả phòng về trạng thái trống
    for (int i = 0; i < MAX_ROOMS; i++) {
        rooms[i].id = -1; // -1 = phòng chưa được tạo/đang trống
        rooms[i].player_count = 0;
        rooms[i].status = ROOM_WAITING;
    }
}

/**
 * Tạo phòng mới
 * @param user_id: ID của người tạo phòng
 * @param username: Tên người dùng
 * @param socket_fd: File descriptor của socket để giao tiếp
 * @param room_name: Tên phòng (chứa game mode: "TenPhong:Mode")
 * @return: Room ID nếu thành công, -1 nếu user đã ở phòng khác, -2 nếu server đầy
 * 
 * FLOW:
 * 1. Kiểm tra user đã ở phòng nào chưa
 * 2. Tìm slot trống trong mảng rooms
 * 3. Parse room_name để lấy tên và game mode
 * 4. Khởi tạo phòng với user làm host (member[0])
 */

int room_create(int user_id, char *username, int socket_fd, char *room_name) {
    // KIỂM TRA: Nếu user đã ở phòng khác, tự động leave trước
    Room *existing = room_get_by_user(user_id);
    if (existing != NULL) {
        printf("[ROOM] User %d already in room, auto-leaving...\n", user_id);
        room_leave(user_id); // Tự động rời phòng cũ
    } 

    // Tìm slot trống để tạo phòng mới
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].id == -1) { // Phòng trống
            rooms[i].id = i; // Gán ID = index
            
            // PARSE PAYLOAD: Format "TenPhong:Mode"
            // "MyRoom:2" -> name="MyRoom", mode=2
            char *p = strchr(room_name, ':'); // Tìm dấu ':'
            if (p) {
                *p = '\0'; // Cắt chuỗi tại vị trí ':'
                strncpy(rooms[i].name, room_name, sizeof(rooms[i].name));
                rooms[i].game_mode = atoi(p + 1); // Parse số sau ':'
            } else {
                strncpy(rooms[i].name, room_name, sizeof(rooms[i].name));
                rooms[i].game_mode = 1; // Mặc định: Mode Elimination
            }
            
            // GAME MODES:
            // 0: Cổ Điển (Chơi đơn, mốc an toàn 5 & 10, có Walk Away)
            // 1: Loại Trừ (Multi-player, ai đúng next câu, cộng điểm)
            // 2: Tính Điểm (Multi-player, tất cả trả lời, điểm cao thắng)
            
            rooms[i].status = ROOM_WAITING; // Trạng thái chờ người chơi
            rooms[i].player_count = 1; // Có 1 người (host)
            
            // KHỞI TẠO HOST (member đầu tiên)
            rooms[i].members[0].user_id = user_id;
            rooms[i].members[0].socket_fd = socket_fd;
            strncpy(rooms[i].members[0].username, username, 50);
            rooms[i].members[0].is_host = 1; // Đánh dấu là chủ phòng
            rooms[i].members[0].score = 0;
            rooms[i].members[0].is_eliminated = 0;

            // Khởi tạo trạng thái trợ giúp (chưa dùng)
            rooms[i].members[0].help_5050_used = 0;
            rooms[i].members[0].help_audience_used = 0;
            rooms[i].members[0].help_phone_used = 0;
            rooms[i].members[0].help_expert_used = 0;
            
            // Reset shared flags (Coop Mode)
            rooms[i].shared_help_5050_used = 0;
            rooms[i].shared_help_audience_used = 0;
            rooms[i].shared_help_phone_used = 0;
            rooms[i].shared_help_expert_used = 0;

            
            printf("[ROOM] Created Room %d: %s by %s\n", i, room_name, username);
            return i; // Trả về Room ID
        }
    }
    return -2; // Server đầy, không còn slot trống
}

/**
 * Tham gia phòng
 * @param room_id: ID của phòng muốn vào
 * @param user_id: ID người dùng
 * @param username: Tên người dùng
 * @param socket_fd: Socket để giao tiếp
 * @return: 1 nếu thành công, âm nếu lỗi
 * 
 * ERROR CODES:
 * -1: Phòng không tồn tại
 * -2: Phòng đầy
 * -3: Phòng đang chơi/kết thúc
 * -4: User đã ở trong phòng rồi
 */
int room_join(int room_id, int user_id, char *username, int socket_fd) {
    // VALIDATE room_id
    if (room_id < 0 || room_id >= MAX_ROOMS) return -1;
    Room *r = &rooms[room_id];
    
    // KIỂM TRA ĐIỀU KIỆN
    if (r->id == -1) return -1; // Phòng chưa được tạo
    if (r->player_count >= MAX_PLAYERS_PER_ROOM) return -2; // Phòng đầy
    if (r->status != ROOM_WAITING) return -3; // Phòng đang chơi hoặc đã kết thúc
    
    // MODE 0 (Cổ Điển): Chỉ cho phép 1 người chơi
    if (r->game_mode == MODE_CLASSIC && r->player_count >= 1) {
        return -5; // Reject: Phòng chơi đơn
    }
    
    // Kiểm tra user đã ở trong phòng chưa (tránh duplicate)
    for(int i=0; i<r->player_count; i++) {
        if (r->members[i].user_id == user_id) return -4; 
    }
    
    // THÊM MEMBER MỚI
    int idx = r->player_count; // Index của member mới
    r->members[idx].user_id = user_id;
    r->members[idx].socket_fd = socket_fd;
    strncpy(r->members[idx].username, username, 50);
    r->members[idx].is_host = 0; // Không phải host
    r->members[idx].score = 0;
    r->members[idx].is_eliminated = 0;

    // Khởi tạo trạng thái trợ giúp
    r->members[idx].help_5050_used = 0;
    r->members[idx].help_audience_used = 0;
    r->members[idx].help_phone_used = 0;  
    r->members[idx].help_expert_used = 0;
    
    r->player_count++; // Tăng số lượng người chơi
    
    printf("[ROOM]: room_join called for UserID %d. Received Username: '%s'\n", user_id, username);
    printf("[ROOM]: Stored member username: '%s'\n", r->members[idx].username);

    printf("[ROOM] User %s joined Room %d\n", username, room_id);
    return 1; // Thành công
}

/**
 * Rời khỏi phòng
 * @param user_id: ID người dùng muốn rời
 * @return: 1 nếu thành công, 0 nếu lỗi
 * 
 * LOGIC:
 * - Nếu HOST rời -> chuyển quyền host cho người thứ 2, game tiếp tục
 * - Nếu member thường rời -> chỉ xóa member đó
 * - Nếu phòng trống -> reset phòng
 */
int room_leave(int user_id) {
    // Tìm phòng của user
    Room *r = room_get_by_user(user_id);
    if (!r) return 0; // User không ở phòng nào

    // Tìm index của user trong mảng members
    int idx = -1;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) {
            idx = i;
            break;
        }
    }

    if (idx != -1) {
        // TRƯỜNG HỢP 1: HOST THOÁT
        // -> Chuyển quyền host cho người thứ 2, game tiếp tục
        if (r->members[idx].is_host) {
            printf("[ROOM] Host %s left room %d.\n", 
                   r->members[idx].username, r->id);
            
            // Nếu chỉ còn 1 người (host) -> Reset phòng
            if (r->player_count == 1) {
                r->id = -1;
                r->player_count = 0;
                r->status = ROOM_WAITING;
                return 1;
            }
            
            // Còn >= 2 người: Chuyển host cho người thứ 2
            // Xóa host cũ (idx=0) bằng cách shift mảng
            for (int i = 0; i < r->player_count - 1; i++) {
                r->members[i] = r->members[i+1];
            }
            r->player_count--;
            
            // Người ở vị trí [0] mới (trước đây là [1]) tự động thành host
            r->members[0].is_host = 1;
            
            printf("[ROOM] Host transferred to %s. Room continues.\n", 
                   r->members[0].username);
            return 1;
        }
        
        // TRƯỜNG HỢP 2: MEMBER THƯỜNG THOÁT
        // -> Xóa member bằng cách shift mảng
        for (int i = idx; i < r->player_count - 1; i++) {
            r->members[i] = r->members[i+1]; // Copy member sau lên trước
        }
        r->player_count--;
        
        // Nếu phòng trống sau khi xóa -> Reset
        if (r->player_count == 0) {
            r->id = -1;
            r->status = ROOM_WAITING;
        }
    }
    
    printf("[ROOM] User %d left room\n", user_id);
    return 1;
}

/**
 * Bắt đầu game (chỉ host mới được gọi)
 * @param room_id: ID phòng
 * @param user_id: ID người gọi (phải là host)
 * @return: 1 nếu thành công, âm nếu lỗi
 * 
 * FLOW:
 * 1. Kiểm tra quyền host
 * 2. Chuyển status sang PLAYING
 * 3. Reset điểm và trạng thái của tất cả members
 * 4. Load 15 câu hỏi ngẫu nhiên từ DB
 * 5. Bắt đầu đếm thời gian cho câu hỏi đầu tiên
 */
int room_start_game(int room_id, int user_id) {
    // VALIDATE
    if (room_id < 0 || room_id >= MAX_ROOMS) return 0;
    Room *r = &rooms[room_id];
    
    // KIỂM TRA QUYỀN: Chỉ host (member[0]) mới được start
    if (r->members[0].user_id != user_id) return -1;
    
    // KIỂM TRA SỐ NGƯỜI CHƠI theo mode
    if (r->game_mode == MODE_CLASSIC) {
        // Mode 0: Allow 1 player
        if (r->player_count < 1) return -2;
    } else {
        // Mode 1 & 2: Require >= 2 players
        if (r->player_count < 2) return -4;
    }

    // CHUYỂN TRẠNG THÁI
    r->status = ROOM_PLAYING;
    r->current_question_idx = 0; // Bắt đầu từ câu hỏi 0
    
    // RESET ĐIỂM & TRẠNG THÁI cho tất cả members
    for(int i=0; i<r->player_count; i++) {
        r->members[i].score = 0;
        r->members[i].is_eliminated = 0; // Chưa bị loại
        r->members[i].has_answered = 0;  // Chưa trả lời (Mode 2)
    }
    
    // Reset Shared Flags for Coop
    r->shared_help_5050_used = 0;
    r->shared_help_audience_used = 0;
    r->shared_help_phone_used = 0;
    r->shared_help_expert_used = 0;


    // LOAD 15 CÂU HỎI NGẪU NHIÊN từ database
    // load_room_questions() sử dụng "ORDER BY RANDOM()" trong SQL
    if (load_room_questions(global_db, r->questions) < 15) {
        printf("[ERROR] Failed to load 15 questions for room %d\n", room_id);
    }

    // BẮT ĐẦU TIMER cho câu hỏi đầu tiên
    r->question_start_time = time(NULL);
    printf("[ROOM] Room %d started game by %d\n", room_id, user_id);
    
    // Khởi tạo game log (lưu lịch sử trả lời)
    r->game_log[0] = '\0';
    
    return 1;
}

/**
 * Lấy thông tin phòng theo ID
 * @param room_id: ID phòng
 * @return: Con trỏ đến Room hoặc NULL nếu không tồn tại
 */
Room* room_get_by_id(int room_id) {
    if (room_id < 0 || room_id >= MAX_ROOMS) return NULL;
    if (rooms[room_id].id == -1) return NULL; // Phòng chưa được tạo
    return &rooms[room_id];
}

/**
 * Tìm phòng mà user đang ở
 * @param user_id: ID người dùng
 * @return: Con trỏ đến Room hoặc NULL nếu user không ở phòng nào
 * 
 * NOTE: Duyệt qua tất cả phòng và tất cả members
 */
Room* room_get_by_user(int user_id) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].id != -1) { // Phòng đang hoạt động
            for (int j = 0; j < rooms[i].player_count; j++) {
                if (rooms[i].members[j].user_id == user_id) {
                    return &rooms[i];
                }
            }
        }
    }
    return NULL; // Không tìm thấy
}

/**
 * Cập nhật timer và tự động chuyển câu hỏi khi hết giờ
 * @param room_id: ID phòng
 * @return: 0 (chưa hết giờ), 1 (chuyển câu), 2 (game over)
 * 
 * LOGIC:
 * - Tính thời gian đã trôi qua từ lúc câu hỏi bắt đầu
 * - Nếu > QUESTION_DURATION (30s) -> chuyển câu tiếp theo
 * - Nếu hết 15 câu -> game kết thúc
 */
int room_update_timer(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->status != ROOM_PLAYING) return 0;

    // Tính thời gian đã trôi qua (giây)
    double elapsed = difftime(time(NULL), r->question_start_time);
    
    // MODE 1: COOP - Timeout = Cả team thua (giống trả lời sai)
    if (r->game_mode == MODE_COOP && elapsed > QUESTION_DURATION) {
        printf("[TIMEOUT] Room %d - Time's up! Team eliminated.\n", room_id);
        
        // Đánh dấu tất cả là eliminated
        for (int i = 0; i < r->player_count; i++) {
            r->members[i].is_eliminated = 1;
        }
        
        r->status = ROOM_FINISHED;
        return 2; // Game Over - trigger broadcast_end_game
    }
    
    // MODE 2: Timeout handling (30s)
    if (r->game_mode == MODE_SCORE_ATTACK && elapsed > QUESTION_DURATION) {
        // Quá thời gian thì tự động là sai
        int any_auto_answered = 0;
        for (int i = 0; i < r->player_count; i++) {
            if (!r->members[i].is_eliminated && !r->members[i].has_answered) {
                r->members[i].has_answered = 1;
                any_auto_answered = 1;
                printf("[TIMEOUT] Player %s auto-marked wrong\n", r->members[i].username);
            }
        }
        
        // Sau khi trả lời auto -> Kiểm tra xem tất cả đã trả lời chưa
        if (any_auto_answered && all_players_answered(r->id)) {
            // Chuyển sang câu hỏi tiếp theo
            r->current_question_idx++;
            reset_answer_flags(r->id); // Cho mode 2
            
            if (r->current_question_idx >= 15) {
                r->status = ROOM_FINISHED;
                return 2; // Game Over
            }
            
            r->question_start_time = time(NULL);
            return 1; // Advance question
        }
    }
    
    // MODE 0: Timeout bình thường (30s) - chỉ chuyển câu
    if (r->game_mode == MODE_CLASSIC && elapsed > QUESTION_DURATION) {
        r->current_question_idx++;
        r->question_start_time = time(NULL);
        
        if (r->current_question_idx >= 15) {
            r->status = ROOM_FINISHED;
            return 2;
        }
        
        return 1; // Chuyển câu hỏi
    }
    
    return 0; // Chưa hết giờ
}

/**
 * Sử dụng trợ giúp (lifeline)
 * @param room_id: ID phòng
 * @param user_id: ID người dùng
 * @param lifeline_type: Loại trợ giúp (1=50:50, 2=Audience, 3=Phone, 4=Expert)
 * @param result_msg: Buffer để lưu kết quả
 * @return: 1 nếu thành công, -1 nếu lỗi
 * 
 * LIFELINE TYPES:
 * 1: 50:50 - Loại 2 đáp án sai
 * 2: Audience Poll - Thống kê ý kiến khán giả
 * 3: Phone Friend - Gọi điện hỏi người thân
 * 4: Expert Advice - Tư vấn từ chuyên gia
 * 
 * NOTE: Mỗi loại chỉ được dùng 1 lần/người/game
 */
int room_use_lifeline(int room_id, int user_id, int lifeline_type, char *result_msg) {
    // VALIDATE
    Room *r = room_get_by_id(room_id);
    if (!r || r->status != ROOM_PLAYING) return -1;
    
    // Tìm member trong phòng
    RoomMember *p = NULL;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) { 
            p = &r->members[i]; 
            break; 
        }
    }
    
    if (!p) return -1; // User không ở trong phòng

    // Lấy câu hỏi hiện tại
    Question *q = &r->questions[r->current_question_idx];

    // XỬ LÝ THEO LOẠI TRỢ GIÚP
    // Cờ để kiểm tra xem đã dùng chưa
    int *flag_used = NULL;
    
    if (r->game_mode == MODE_COOP) {
        // Mode Coop: Dùng flag chung của phòng
        if (lifeline_type == 1) flag_used = &r->shared_help_5050_used;
        else if (lifeline_type == 2) flag_used = &r->shared_help_audience_used;
        else if (lifeline_type == 3) flag_used = &r->shared_help_phone_used;
        else if (lifeline_type == 4) flag_used = &r->shared_help_expert_used;
    } else {
        // Mode Khác: Dùng flag riêng của member
        if (lifeline_type == 1) flag_used = &p->help_5050_used;
        else if (lifeline_type == 2) flag_used = &p->help_audience_used;
        else if (lifeline_type == 3) flag_used = &p->help_phone_used;
        else if (lifeline_type == 4) flag_used = &p->help_expert_used;
    }
    
    if (flag_used == NULL) {
         strcpy(result_msg, "Loại trợ giúp không hợp lệ.");
         return -1;
    }
    
    // Kiểm tra và đánh dấu
    if (*flag_used == 1) {
        if (r->game_mode == MODE_COOP) {
            strcpy(result_msg, "Quyền trợ giúp này đã được sử dụng!");
        } else {
            strcpy(result_msg, "Bạn đã sử dụng quyền trợ giúp này rồi!");
        }
        return 1; // Handled but denied
    }
    
    *flag_used = 1; // Đánh dấu đã dùng

    // Gọi logic lấy nội dung trợ giúp
    if (lifeline_type == 1) {
        get_5050_options(q, result_msg); 
    } 
    else if (lifeline_type == 2) {
        get_audience_stats(q, result_msg);
    }
    else if (lifeline_type == 3) {
        get_phone_friend_response(q, result_msg);
    }
    else if (lifeline_type == 4) {
        get_expert_advice(q, result_msg);
    } 

    return 1;
}

/**
 * Lấy danh sách tất cả các phòng dạng chuỗi
 * @param buffer: Buffer để lưu kết quả
 * 
 * FORMAT: "id:name:count:status,id2:name2:count2:status2,..."
 * VD: "0:MyRoom:3:1,1:TestRoom:1:0"
 * 
 * FIELDS:
 * - id: Room ID
 * - name: Tên phòng
 * - count: Số người chơi hiện tại
 * - status: Trạng thái (0=WAITING, 1=PLAYING, 2=FINISHED)
 */
void room_get_list_string(char *buffer) {
    buffer[0] = '\0'; // Khởi tạo chuỗi rỗng
    
    // Duyệt qua tất cả các phòng
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].id != -1) { // Phòng đang hoạt động
            // BỎ QUA phòng Mode 0 (Cổ Điển) - chỉ chơi đơn, không hiển thị
            if (rooms[i].game_mode == MODE_CLASSIC) continue;
            
            char temp[128];
            sprintf(temp, "%d:%s:%d:%d,", 
                rooms[i].id, 
                rooms[i].name, 
                rooms[i].player_count, 
                rooms[i].status
            );
            strcat(buffer, temp);
        }
    }
    
    // Xóa dấu phây cuối cùng
    int list_len = strlen(buffer);
    if (list_len > 0) buffer[list_len - 1] = '\0';
}

/**
 * Lấy chi tiết members trong phòng
 * @param room_id: ID phòng
 * @param buffer: Buffer để lưu kết quả
 * 
 * FORMAT: "is_host:username:score,is_host:username:score,..."
 * VD: "1:Alice:1000,0:Bob:500"
 * 
 * FIELDS:
 * - is_host: 1 nếu là host, 0 nếu là member thường
 * - username: Tên người chơi
 * - score: Điểm/tiền hiện tại
 */
void room_get_detail_string(int room_id, char *buffer) {
    buffer[0] = '\0';
    Room *r = room_get_by_id(room_id);
    if (!r) return; // Phòng không tồn tại
    
    // Duyệt qua tất cả members
    for (int i = 0; i < r->player_count; i++) {
        char temp[128];
        sprintf(temp, "%d:%s:%d,", 
            r->members[i].is_host,
            r->members[i].username,
            r->members[i].score
        );
        strcat(buffer, temp);
    }
    
    // Xóa dấu phẩy cuối
    int len = strlen(buffer);
    if (len > 0) buffer[len - 1] = '\0';
}

/**
 * ===================================
 * MODE-SPECIFIC ANSWER HANDLERS
 * ===================================
 */

/**
 * MODE 0: LUYỆN TẬP (Practice Mode)
 * - Single player
 * - Fixed prize (overwrite score)
 * - Safe-haven on wrong answer
 * 
 * @return: 1 (correct), 2 (eliminated)
 */
int room_handle_answer_practice(int user_id, char *answer, char *result_msg) {
    Room *r = room_get_by_user(user_id);
    if (!r || r->status != ROOM_PLAYING) {
        strcpy(result_msg, "Lỗi: phòng không chơi hoặc bạn chưa vào phòng.");
        return -1;
    }
    
    // Tìm index của user
    int idx = -1;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1 || r->members[idx].is_eliminated) {
        strcpy(result_msg, "Bạn đã bị loại hoặc không trong phòng.");
        return 0;
    }
    
    // Lấy câu hỏi hiện tại
    Question *q = &r->questions[r->current_question_idx];
    double elapsed = difftime(time(NULL), r->question_start_time);
    
    // KIỂM TRA ĐÁP ÁN
    int res = calculate_score(q, answer, elapsed);
    
    // LOGGING
    char log_entry[64];
    sprintf(log_entry, "%d:%d:%s,", user_id, q->id, answer);
    if (strlen(r->game_log) + strlen(log_entry) < 4095) {
        strcat(r->game_log, log_entry);
    }
    
    int current_level = r->current_question_idx + 1;
    
    if (res == 1) {
        // Đúng: Ghi đè điểm theo level
        int prize = get_prize_for_level(current_level);
        r->members[idx].score = prize;
        sprintf(result_msg, "Đúng rồi! Bạn đang ở mức câu hỏi %d. Tiền: %d. Tổng: %d", 
                current_level, prize, r->members[idx].score);
        broadcast_scores(r->id);
        return 1; // Đúng
    } else {
        // Sai: Bị loại, về mốc an toàn
        int safe_money = calculate_safe_reward(current_level);
        r->members[idx].score = safe_money;
        r->members[idx].is_eliminated = 1;
        sprintf(result_msg, "Sai rồi! Bạn bị loại, tiền an toàn: %d. Tổng: %d", safe_money, r->members[idx].score);
        broadcast_scores(r->id);
        return 2; // Bị loại
    }
}

/**
 * MODE 1: COOP (Hợp tác)
 * - Multiplayer (2-4 người)
 * - Trả lời ĐÚNG -> Cả team next câu
 * - Trả lời SAI -> Cả team THUA
 * - Chung quyền trợ giúp
 * 
 * @return: 3 (correct + instant advance), 2 (eliminated entire team)
 */
int room_handle_answer_coop(int user_id, char *answer, char *result_msg) {
    Room *r = room_get_by_user(user_id);
    if (!r || r->status != ROOM_PLAYING) {
        strcpy(result_msg, "Lỗi: phòng không chơi hoặc bạn chưa vào phòng.");
        return -1;
    }
    
    // Tìm index của user
    int idx = -1;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1 || r->members[idx].is_eliminated) {
        strcpy(result_msg, "Lỗi: Bạn đã bị loại hoặc không trong phòng.");
        return 0;
    }
    
    // Lấy câu hỏi hiện tại
    Question *q = &r->questions[r->current_question_idx];
    double elapsed = difftime(time(NULL), r->question_start_time);
    
    // KIỂM TRA ĐÁP ÁN
    int res = calculate_score(q, answer, elapsed);
    
    // LOGGING
    char log_entry[64];
    sprintf(log_entry, "%d:%d:%s,", user_id, q->id, answer);
    if (strlen(r->game_log) + strlen(log_entry) < 4095) {
        strcat(r->game_log, log_entry);
    }
    
    int current_level = r->current_question_idx + 1;
    
    if (res == 1) {
        // --- TRƯỜNG HỢP ĐÚNG ---
        // Cộng điểm cho người trả lời (tích lũy cá nhân)
        int prize = get_prize_for_level(current_level);
        int prev_prize = (current_level > 1) ? get_prize_for_level(current_level - 1) : 0;
        int diff = prize - prev_prize;
        
        r->members[idx].score += diff;
        sprintf(result_msg, "CHÍNH XÁC: Đồng đội đã đưa cả đội lên câu hỏi tiếp theo (+%d điểm)", diff);
        broadcast_scores(r->id);
        
        printf("[COOP] Player %s correct → Team advances\n", r->members[idx].username);
        return 3; // Correct + Instant Advance (cho cả team)
    } else {
        // --- TRƯỜNG HỢP SAI ---
        // Cả team bị loại ngay lập tức
        printf("[COOP] Player %s wrong → TEAM ELIMINATED\n", r->members[idx].username);
        
        // Đánh dấu tất cả là eliminated
        for(int i=0; i<r->player_count; i++) {
            r->members[i].is_eliminated = 1;
        }

        sprintf(result_msg, "SAI RỒI! Cả đội đã bị loại tại câu số %d. Game Over!", current_level);
        broadcast_scores(r->id);
        
        // Return 2 để trigger broadcast_end_game
        return 2;
    }
}

/**
 * MODE 2: TỐC ĐỘ (Speed Attack Mode)
 * - Multiplayer (≥2)
 * - Cumulative prize + time bonus
 * - No elimination on wrong
 * - Track has_answered
 * 
 * @return: 4 (all answered → advance), 1 (correct, wait), 0 (wrong, wait)
 */
int room_handle_answer_speedattack(int user_id, char *answer, char *result_msg) {
    Room *r = room_get_by_user(user_id);
    if (!r || r->status != ROOM_PLAYING) {
        strcpy(result_msg, "Lỗi: phòng không chơi hoặc bạn chưa vào phòng.");
        return -1;
    }
    
    // Tìm index của user
    int idx = -1;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1 || r->members[idx].is_eliminated) {
        strcpy(result_msg, "Lỗi: Bạn đã bị loại hoặc không trong phòng.");
        return 0;
    }
    
    // Check if already answered
    if (r->members[idx].has_answered) {
        strcpy(result_msg, "Lỗi: Bạn đã trả lời câu này rồi!");
        return 0;
    }
    
    // Lấy câu hỏi hiện tại
    Question *q = &r->questions[r->current_question_idx];
    double elapsed = difftime(time(NULL), r->question_start_time);
    
    // KIỂM TRA ĐÁP ÁN
    int res = calculate_score(q, answer, elapsed);
    
    // LOGGING
    char log_entry[64];
    sprintf(log_entry, "%d:%d:%s,", user_id, q->id, answer);
    if (strlen(r->game_log) + strlen(log_entry) < 4095) {
        strcat(r->game_log, log_entry);
    }
    
    // Mark as answered
    r->members[idx].has_answered = 1;
    
    int current_level = r->current_question_idx + 1;
    
    if (res == 1) {
        // Đúng: Cộng dồn chênh lệch + time bonus
        int prize = get_prize_for_level(current_level);
        int prev_prize = (current_level > 1) ? get_prize_for_level(current_level - 1) : 0;
        int diff = prize - prev_prize;
        
        int time_bonus = (30 - (int)elapsed);
        if (time_bonus < 0) time_bonus = 0;
        
        int total = diff + (time_bonus * 10);
        r->members[idx].score += total;
        
        sprintf(result_msg, "CHÍNH XÁC! +%d điểm (base: %d, bonus: %d). Tổng: %d", 
                total, diff, time_bonus * 10, r->members[idx].score);
        broadcast_scores(r->id);
        
        // Check if all answered
        if (all_players_answered(r->id)) {
            printf("[MODE2] All answered → Advancing\n");
            return 4; // All answered, advance
        }
        return 1; // Correct, waiting for others
    } else {
        // Sai: Không loại, không điểm
        sprintf(result_msg, "SAI ROI! Bạn không được điểm. Tổng: %d", r->members[idx].score);
        broadcast_scores(r->id);
        
        // Check if all answered
        if (all_players_answered(r->id)) {
            printf("[MODE2] All answered (including wrong) → Advancing\n");
            return 4; // All answered, advance
        }
        return 0; // Wrong, waiting for others
    }
}

/**
 * ===================================
 * DISPATCHER
 * ===================================
 * Calls appropriate mode-specific handler
 * 
 * @return: Mode-specific return codes (see each handler)
 */
int room_handle_answer(int user_id, char *answer, char *result_msg) {
    Room *r = room_get_by_user(user_id);
    if (!r || r->status != ROOM_PLAYING) {
        strcpy(result_msg, "Lỗi: phòng không chơi hoặc bạn chưa vào phòng.");
        return -1;
    }
    
    // Dispatch to mode-specific handler
    switch (r->game_mode) {
        case MODE_CLASSIC:
            return room_handle_answer_practice(user_id, answer, result_msg);
        case MODE_COOP:
            return room_handle_answer_coop(user_id, answer, result_msg);
        case MODE_SCORE_ATTACK:
            return room_handle_answer_speedattack(user_id, answer, result_msg);
        default: break;
    }
    
    strcpy(result_msg, "Lỗi: Chế độ không hợp lệ.");
    return -1;
}

/**
 * Dừng cuộc chơi (Walk Away)
 * @param result_msg: Buffer để lưu kết quả
 * @return: 1 nếu thành công, 0 nếu lỗi
 * 
 * LOGIC:
 * - Người chơi giữ nguyên số điểm/tiền hiện tại
 * - Đánh dấu is_eliminated = 1 (không tham gia câu sau)
 * - Không ảnh hưởng đến người chơi khác
 * 
 * USE CASE: Người chơi cảm thấy câu hỏi quá khó, muốn bảo toàn điểm
 */
int room_walk_away(int user_id, char *result_msg) {
    // Tìm phòng của user
    Room *r = room_get_by_user(user_id);
    if (!r || r->status != ROOM_PLAYING) return 0;
    
    // Tìm member
    int idx = -1;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) { 
            idx = i; 
            break; 
        }
    }
    
    // VALIDATE: Member phải còn trong game
    if (idx == -1 || r->members[idx].is_eliminated) return 0;

    // DỪNG CUỘC CHƠI
    // Giữ nguyên điểm hiện tại, không bị phạt
    r->members[idx].is_eliminated = 1; // Đánh dấu không chơi nữa
    
    int final_score = r->members[idx].score;

    sprintf(result_msg, "Bạn đã dừng cuộc chơi. Tổng tiền thưởng: %d", final_score);
    
    // Broadcast score update one last time
    broadcast_scores(r->id);
    
    // === CHECK IF GAME SHOULD END ===
    int survivors_count = 0;
    
    // Check remaining active players
    for (int i = 0; i < r->player_count; i++) {
        if (!r->members[i].is_eliminated) {
            survivors_count++;
        }
    }
    
    // MODE 1 (Coop): If count == 0 (all eliminated/walked away) -> End game
    if (r->game_mode == MODE_COOP && survivors_count == 0) {
        printf("[WALK_AWAY] Mode 1: All players eliminated/left - Auto-ending game\n");
        broadcast_end_game(r->id, global_db);
    }
    // MODE 2 (Score Attack): If ALL players walked away, end game and find winner by score
    else if (r->game_mode == MODE_SCORE_ATTACK && survivors_count == 0) {
        printf("[WALK_AWAY] Mode 2: All players walked away - Ending game\n");
        broadcast_end_game(r->id, global_db);
    }
    
    return 1;
}

/*
 * ============================================================
 * TÓM TẮT CẤU TRÚC DỮ LIỆU
 * ============================================================
 * 
 * Room {
 *   int id;                    // ID phòng (hoặc -1 nếu trống)
 *   char name[64];             // Tên phòng
 *   int game_mode;             // 0=Practice, 1=Elimination, 2=Score
 *   RoomStatus status;         // WAITING/PLAYING/FINISHED
 *   
 *   RoomMember members[MAX];   // Mảng người chơi
 *   int player_count;          // Số người hiện tại
 *   
 *   Question questions[15];    // 15 câu hỏi cho game
 *   int current_question_idx;  // Câu hỏi hiện tại (0-14)
 *   time_t question_start_time;// Thời điểm bắt đầu câu hỏi
 *   
 *   char game_log[4096];       // Lịch sử trả lời
 * }
 * 
 * RoomMember {
 *   int user_id;               // ID từ DB
 *   int socket_fd;             // Socket để gửi/nhận
 *   char username[50];         // Tên hiển thị
 *   int is_host;               // 1=host, 0=member
 *   int score;                 // Điểm/tiền hiện tại
 *   int is_eliminated;         // 1=đã loại/dừng
 *   
 *   // Trạng thái trợ giúp
 *   int help_5050_used;
 *   int help_audience_used;
 *   int help_phone_used;
 *   int help_expert_used;
 * }
 * 
 * ============================================================
 * FLOW CHO MỘT GAME HOÀN CHỈNH
 * ============================================================
 * 
 * 1. KHỞI TẠO SERVER
 *    room_system_init(db_conn)
 * 
 * 2. TẠO PHÒNG
 *    room_create(user_id, username, socket_fd, "MyRoom:1")
 *    -> Phòng có ID, host là member[0]
 * 
 * 3. NGƯỜI CHƠI THAM GIA
 *    room_join(room_id, user_id2, username2, socket_fd2)
 *    -> Thêm vào members[]
 * 
 * 4. BẮT ĐẦU GAME (Host)
 *    room_start_game(room_id, host_user_id)
 *    -> Load 15 câu hỏi
 *    -> Status = PLAYING
 *    -> Bắt đầu timer
 * 
 * 5. VÒNG LẶP GAME (Cho mỗi câu hỏi)
 *    a) Server gửi câu hỏi cho tất cả members
 *    b) Members trả lời:
 *       - room_handle_answer(user_id, "A", result_msg)
 *       - Hoặc room_use_lifeline(room_id, user_id, 1, result_msg)
 *       - Hoặc room_walk_away(user_id, result_msg)
 *    c) room_update_timer(room_id)
 *       - Nếu hết giờ -> chuyển câu
 *       - Nếu hết 15 câu -> Status = FINISHED
 * 
 * 6. KẾT THÚC GAME
 *    - Kiểm tra r->status == ROOM_FINISHED
 *    - Lưu kết quả vào DB
 *    - Thông báo cho tất cả members
 *    - Reset phòng hoặc để người chơi leave
 * 
 * ============================================================
 * CÁC HÀM TIỆN ÍCH
 * ============================================================
 * 
 * room_get_by_id(room_id)      -> Tìm phòng theo ID
 * room_get_by_user(user_id)    -> Tìm phòng user đang ở
 * room_get_list_string(buffer) -> Danh sách phòng
 * room_get_detail_string(id, buffer) -> Chi tiết members
 * 
 * ============================================================
 */

/**
 * Kiểm tra xem tất cả người chơi trong phòng đã bị loại chưa
 * @param room_id: ID phòng
 * @return: 1 nếu tất cả bị loại, 0 nếu còn người chơi
 */
int room_all_eliminated(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->status != ROOM_PLAYING) return 0;
    
    int active_players = 0;
    for (int i = 0; i < r->player_count; i++) {
        if (!r->members[i].is_eliminated) {
            active_players++;
        }
    }
    
    return (active_players == 0) ? 1 : 0;
}

// void room_remove_player(int room_id, int user_id) {
//     Room *r = room_get_by_id(room_id);
//     if (!r) return;
    
//     // Find player index
//     int idx = -1;
//     for (int i = 0; i < r->player_count; i++) {
//         if (r->members[i].user_id == user_id) {
//             idx = i;
//             break;
//         }
//     }
    
//     if (idx == -1) return; // Player not found
    
//     // Check if player is host
//     int was_host = r->members[idx].is_host;
    
//     // Shift remaining players down
//     for (int i = idx; i < r->player_count - 1; i++) {
//         r->members[i] = r->members[i + 1];
//     }
    
//     // Decrease count
//     r->player_count--;
    
//     // Transfer host to first player if needed
//     if (was_host && r->player_count > 0) {
//         r->members[0].is_host = 1;
//         printf("[ROOM] Host transferred to %s\n", r->members[0].username);
//     }
    
//     printf("[ROOM] Player removed. Room %d now has %d players\n", room_id, r->player_count);
// }

/**
 * Kiểm tra xem tất cả người chơi đã trả lời câu hiện tại (Mode 2)
 * @return: 1 nếu tất cả đã trả lời, 0 nếu còn người chưa trả lời
 */
int all_players_answered(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r || r->status != ROOM_PLAYING) return 0;
    
    for (int i = 0; i < r->player_count; i++) {
        if (!r->members[i].is_eliminated && !r->members[i].has_answered) {
            return 0; // Found player who hasn't answered
        }
    }
    
    return 1; // All answered
}

/**
 * Reset trạng thái trả lời cho câu mới (Mode 2)
 */
void reset_answer_flags(int room_id) {
    Room *r = room_get_by_id(room_id);
    if (!r) return;
    
    for (int i = 0; i < r->player_count; i++) {
        r->members[i].has_answered = 0;
    }
}