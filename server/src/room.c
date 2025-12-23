#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/room.h"

// --- KHAI BÁO THƯ VIỆN ĐỂ SEND ---
#include <sys/socket.h>
#include <unistd.h>
#include "../include/protocol.h"

// --- KHAI BÁO CÁC HÀM TỪ GAME.C ---
// Các hàm trợ giúp từ module game
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
 * @param room_name: Tên phòng (có thể chứa game mode: "TenPhong:Mode")
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
            
            // PARSE PAYLOAD: Format "TenPhong:Mode" hoặc chỉ "TenPhong"
            // Ví dụ: "MyRoom:2" -> name="MyRoom", mode=2
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
    
    printf("[ROOM] User %s joined Room %d\n", username, room_id);
    return 1; // Thành công
}

/**
 * Rời khỏi phòng
 * @param user_id: ID người dùng muốn rời
 * @return: 1 nếu thành công, 0 nếu lỗi
 * 
 * LOGIC:
 * - Nếu HOST rời -> kick tất cả members + reset phòng
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
        // -> Chuyển quyền host cho người thứ 2, game/phòng tiếp tục
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
    if (r->player_count < 1) return -2; // Phải có ít nhất 1 người

    // CHUYỂN TRẠNG THÁI
    r->status = ROOM_PLAYING;
    r->current_question_idx = 0; // Bắt đầu từ câu hỏi 0
    
    // RESET ĐIỂM & TRẠNG THÁI cho tất cả members
    for(int i=0; i<r->player_count; i++) {
        r->members[i].score = 0;
        r->members[i].is_eliminated = 0; // Chưa bị loại
    }

    // LOAD 15 CÂU HỎI NGẪU NHIÊN từ database
    // load_room_questions() sử dụng "ORDER BY RANDOM()" trong SQL
    if (load_room_questions(global_db, r->questions) < 15) {
        printf("[ERROR] Failed to load 15 questions for room %d\n", room_id);
        // TODO: Xử lý lỗi tốt hơn (hiện tại vẫn tiếp tục)
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
    
    // KIỂM TRA HẾT GIỜ
    if (elapsed > QUESTION_DURATION) {
        // Chuyển sang câu hỏi tiếp theo
        r->current_question_idx++;
        r->question_start_time = time(NULL); // Reset timer
        
        // KIỂM TRA KẾT THÚC GAME
        if (r->current_question_idx >= 15) {
            r->status = ROOM_FINISHED;
            return 2; // Game Over
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
    if (lifeline_type == 1) {
        // 50:50 - Loại bỏ 2 đáp án sai
        if (p->help_5050_used) { 
            strcpy(result_msg, "Ban da su dung quyen 50:50 roi!"); 
            goto end; 
        }
        p->help_5050_used = 1; // Đánh dấu đã dùng
        get_5050_options(q, result_msg); // Gọi hàm game.c
    } 
    else if (lifeline_type == 2) {
        // Audience Poll - Thống kê % chọn từng đáp án
        if (p->help_audience_used) { 
            strcpy(result_msg, "Ban da su dung quyen Hoi y kien khan gia roi!"); 
            goto end; 
        }
        p->help_audience_used = 1;
        get_audience_stats(q, result_msg);
    }
    else if (lifeline_type == 3) {
        // Phone Friend - Người thân gợi ý đáp án
        if (p->help_phone_used) { 
            strcpy(result_msg, "Ban da su dung quyen Goi dien thoai cho nguoi than roi!"); 
            goto end; 
        }
        p->help_phone_used = 1;
        get_phone_friend_response(q, result_msg);
    }
    else if (lifeline_type == 4) {
        // Expert Advice - Chuyên gia tư vấn
        if (p->help_expert_used) { 
            strcpy(result_msg, "Ban da su dung quyen To tu van tai cho roi!"); 
            goto end; 
        }
        p->help_expert_used = 1;
        get_expert_advice(q, result_msg);
    } else {
        strcpy(result_msg, "Loai tro giup khong hop le.");
    }

end:
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
 * Xử lý câu trả lời của người chơi
 * @param user_id: ID người trả lời
 * @param answer: Câu trả lời (A/B/C/D)
 * @param result_msg: Buffer để lưu kết quả
 * @return: 0 (sai/timeout), 1 (đúng), 2 (bị loại)
 * 
 * LOGIC THEO GAME MODE:
 * 
 * Mode 0 & 1 (Practice/Elimination):
 * - Đúng: Nhận tiền thưởng theo level, tiếp tục chơi
 * - Sai: Ra về với mốc an toàn, bị loại (is_eliminated=1)
 * 
 * Mode 2 (Score Attack):
 * - Đúng: Tích lũy điểm (base 100 + time bonus), tiếp tục
 * - Sai: Không điểm, KHÔNG bị loại, tiếp tục chơi
 * 
 * LOGGING: Ghi lại lịch sử trả lời "UserID:QuestionID:Answer,"
 */
int room_handle_answer(int user_id, char *answer, char *result_msg) {
    // VALIDATE: Tìm phòng của user
    Room *r = room_get_by_user(user_id);
    if (!r || r->status != ROOM_PLAYING) {
        strcpy(result_msg, "Loi: Phong khong choi hoac ban chua vao phong.");
        return 0;
    }

    // Tìm index của user trong members
    int idx = -1;
    for (int i = 0; i < r->player_count; i++) {
        if (r->members[i].user_id == user_id) { 
            idx = i; 
            break; 
        }
    }
    
    // VALIDATE: User phải còn trong game
    if (idx == -1 || r->members[idx].is_eliminated) {
        strcpy(result_msg, "Ban da bi loai hoac khong trong phong.");
        return 0; 
    }

    // Lấy câu hỏi hiện tại
    Question *q = &r->questions[r->current_question_idx];
    
    // Tính thời gian đã trôi qua
    double elapsed = difftime(time(NULL), r->question_start_time);
    
    // KIỂM TRA ĐÁP ÁN qua module game.c
    // calculate_score() return: 1 (đúng), -1 (sai), 0 (timeout)
    int res = calculate_score(q, answer, elapsed);
    
    // LOGGING: Ghi lại câu trả lời
    // Format: "UserID:QuestionID:Answer,"
    char log_entry[64];
    sprintf(log_entry, "%d:%d:%s,", user_id, q->id, answer);
    if (strlen(r->game_log) + strlen(log_entry) < 4095) {
        strcat(r->game_log, log_entry);
    }
    
    // Level hiện tại (1..15)
    int current_level = r->current_question_idx + 1; 

    // === XỬ LÝ KẾT QUẢ ===
    
    if (res == 1) {
        // ✅ CHÍNH XÁC
        int prize = 0;
        
        if (r->game_mode == 2) {
            // MODE 2: Score Attack - Tính điểm theo thời gian
            // Công thức: 100 điểm cơ bản + (30 - thời gian) * 10
            // Trả lời nhanh = điểm cao
            int time_bonus = (30 - (int)elapsed);
            if (time_bonus < 0) time_bonus = 0;
            prize = 100 + time_bonus * 10;
            
            // Cộng dồn điểm
            r->members[idx].score += prize;
        } else {
            // MODE 0 & 1: Practice/Elimination - Tiền thưởng cố định
            prize = get_prize_for_level(current_level);
            
            // Ghi đè điểm (không cộng dồn)
            r->members[idx].score = prize;
        }
        
        sprintf(result_msg, "CHINH XAC! Ban dang o muc cau hoi %d. Diem/Tien: %d", 
                current_level, r->members[idx].score);
        
        return 1; // Trả lời đúng
    } 
    else {
        // ❌ SAI HOẶC HẾT GIỜ
        
        if (r->game_mode == 2) {
             // MODE 2: Score Attack
             // Không bị loại, không mất điểm, tiếp tục chơi
             sprintf(result_msg, "SAI ROI! Ban khong duoc diem cau nay.");
             return 0; // Không bị loại
        } else {
             // MODE 0 & 1: Practice/Elimination
             // Bị loại, giữ mốc tiền an toàn
             
             // Tính mốc an toàn gần nhất
             // VD: Câu 1-5 -> 0đ, Câu 6-10 -> mốc câu 5, Câu 11-15 -> mốc câu 10
             int safe_money = calculate_safe_reward(current_level);
             r->members[idx].score = safe_money;
             r->members[idx].is_eliminated = 1; // ĐÁNH DẤU BỊ LOẠI
             
             sprintf(result_msg, "SAI ROI! Ra ve voi so tien: %d", safe_money);
             return 2; // Bị loại
        }
    }
}

/**
 * Dừng cuộc chơi (Walk Away)
 * @param user_id: ID người muốn dừng
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
    
    sprintf(result_msg, "Ban da dung cuoc choi. Tong tien thuong: %d", 
            r->members[idx].score);
    
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