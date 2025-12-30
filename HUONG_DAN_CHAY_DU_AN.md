# HƯỚNG DẪN CHẠY DỰ ÁN AI LÀ TRIỆU PHÚ

## 🎯 Giới thiệu
Dự án **Ai Là Triệu Phú** là game trắc nghiệm trực tuyến multiplayer được xây dựng bằng C (Server), NodeJS (Middleware) và ReactJS (Client). Game hỗ trợ 3 chế độ chơi khác nhau với hệ thống phòng, điểm số và xếp hạng.

## 📁 Cấu trúc dự án
```
Network/
├── database/           # SQLite database
│   ├── schema.sql     # Cấu trúc bảng
│   └── data.sql       # Dữ liệu mẫu (câu hỏi, user)
├── server/            # Backend C
│   ├── include/       # Header files
│   └── src/          # Source files
├── middleware/        # WebSocket-TCP bridge (NodeJS)
└── client/           # Frontend ReactJS
```

---

## 🚀 Cách chạy dự án

### Bước 1: Khởi tạo Database (Lần đầu hoặc Reset)
```bash
cd database
rm -f database.db                    # Xóa DB cũ (nếu có)
sqlite3 database.db < schema.sql     # Tạo cấu trúc bảng
sqlite3 database.db < data.sql       # Import dữ liệu mẫu
cd ..
```

### Bước 2: Chạy Server (C)
Mở terminal thứ nhất:
```bash
cd server/src
make clean && make    # Compile
./game_app           # Chạy server
```
**Port:** `8080` (TCP)

### Bước 3: Chạy Middleware (NodeJS)
Mở terminal thứ hai:
```bash
cd middleware
npm install          # Lần đầu: cài dependencies
npm start            # Chạy middleware
```
**Port:** `4000` (WebSocket)

### Bước 4: Chạy Client (ReactJS)
Mở terminal thứ ba:
```bash
cd client
npm install          # Lần đầu: cài dependencies
npm run dev          # Chạy dev server
```
**Truy cập:** Mở link hiển thị trong terminal (thường là `http://localhost:5173`)

---

## 🎮 Tính năng chính

### 1. Hệ thống người dùng
- ✅ **Đăng ký/Đăng nhập**: Tài khoản cá nhân với username/password
- ✅ **Phân quyền**: User thường (role=1) và Admin (role=0)
- ✅ **Theo dõi điểm số**: Điểm tích lũy và số trận thắng
- ✅ **Xếp hạng**: Leaderboard theo điểm số

### 2. Hệ thống phòng chơi
- ✅ **Tạo phòng**: Đặt tên và chọn chế độ chơi (0, 1, 2)
- ✅ **Tham gia phòng**: Tối đa 4 người/phòng (tùy mode)
- ✅ **Mời bạn bè**: Gửi lời mời vào phòng
- ✅ **Host quyền**: Chủ phòng có quyền bắt đầu game
- ✅ **Chuyển host**: Tự động chuyển khi host rời phòng

### 3. Hệ thống game
- ✅ **15 câu hỏi/ván**: Tăng dần độ khó (5 Dễ, 5 TB, 5 Khó)
- ✅ **4 quyền trợ giúp** (Lifelines):
  - 🔹 **50:50**: Loại 2 đáp án sai
  - 🔹 **Hỏi ý kiến khán giả**: Thống kê % chọn từng đáp án
  - 🔹 **Gọi điện**: Người thân gợi ý (80% đúng)
  - 🔹 **Chuyên gia**: Tư vấn chính xác 100%
- ✅ **Lưu lịch sử**: Replay các trận đã chơi
- ✅ **Điểm số thời gian thực**: Cập nhật live trong game

### 4. Tính năng Admin
- ✅ Xem danh sách tất cả user
- ✅ Xoá/Khóa user
- ✅ Xem thống kê chi tiết user

---

## 🎲 Chế độ chơi (Game Modes)

### Mode 0: Cổ Điển (Classic) - Chơi đơn
**Đặc điểm:**
- 👤 **1 người chơi**
- 💰 **Thang điểm cố định**: Mỗi câu có giá trị riêng (200 → 150,000)
- 🛡️ **Mốc an toàn**:
  - Câu 5: 2,000 điểm
  - Câu 10: 22,000 điểm
- ❌ **Trả lời sai**: Về mốc an toàn gần nhất (chưa qua câu 5 = 0 điểm)
- 🚶 **Walk Away**: Có thể dừng giữa chừng để giữ tiền
- ✅ **Chiến thắng**: Hoàn thành 15 câu = 150,000 điểm

**Thang điểm:**
```
Câu 1-5:   200, 400, 600, 1,000, 2,000 (Mốc an toàn)
Câu 6-10:  3,000, 6,000, 10,000, 14,000, 22,000 (Mốc an toàn)
Câu 11-15: 30,000, 40,000, 60,000, 85,000, 150,000
```

### Mode 1: Hợp Tác (Coop Mode) - Đồng Đội
**Đặc điểm:**
- 👥 **2-4 người chơi** (Chung 1 team)
- 🤝 **Chung sức leo núi**:
  - Bất kỳ ai trả lời **ĐÚNG** → Cả team qua câu tiếp theo (Advance)
  - Chỉ cần 1 người trả lời **SAI** → Cả team **THUA** ngay lập tức
- 🆘 **Quyền trợ giúp chia sẻ**: 
  - Cả team dùng chung 4 quyền trợ giúp
  - VD: Người A dùng 50:50 → Người B, C, D không thể dùng 50:50 nữa
- 🏆 **Chiến thắng**: Cả team cùng vượt qua câu số 15
- 🎁 **Phần thưởng**:
  - `total_win` +1 cho TẤT CẢ thành viên
  - Điểm số tích lũy cá nhân (ai đúng người đó được điểm)

### Mode 2: Tính Điểm (Speed Attack) - Đua điểm
**Đặc điểm:**
- 👥 **2-4 người chơi**
- ⏳ **Tất cả trả lời**: Đợi tất cả người chơi trả lời mới next câu
- ⭐ **Điểm thưởng thời gian**: `điểm_cơ_bản + (30 - giây_dùng) * 10`
  - VD: Trả lời câu 2 sau 5 giây:
    - Điểm cơ bản: 200 (chênh lệch 400 - 200)
    - Bonus: (30 - 5) * 10 = 250
    - Tổng: +450 điểm
- ❌ **Trả lời sai**: Không điểm, KHÔNG bị loại
- ⏱️ **Timeout**: 30 giây/câu, qua thời gian = tự động SAI
- 🏆 **Chiến thắng**: Điểm cao nhất sau 15 câu

---

## 📊 Database Schema

### Bảng chính:
- **users**: Thông tin user (username, password, role, điểm tích lũy)
- **questions**: Ngân hàng câu hỏi (content, 4 options, đáp án đúng, độ khó)
- **game_history**: Lịch sử các trận đấu (winner, mode, game_log)
- **user_stats**: Thống kê chi tiết từng trận (user_id, game_id, score)

---

## 🔧 Lưu ý quan trọng

### 1. Logs
- File log tự động tạo trong `logs/` khi chạy
- ⚠️ **KHÔNG tạo thủ công** để tránh lỗi encoding
- Files: `server_history.txt`, `middleware_history.txt`

### 2. Database
- File DB: `database/database.db`
- Reset DB: Xóa file và chạy lại schema + data.sql
- Backup trước khi reset nếu cần giữ dữ liệu

### 3. Ports
- Server: `8080` (TCP)
- Middleware: `4000` (WebSocket)
- Client: `5173` (Vite dev server)
- Đảm bảo các port này chưa bị chiếm dụng

### 4. Dependencies
**Server (C):**
- `gcc` compiler
- `sqlite3` library
- `make`

**Middleware (NodeJS):**
- Node.js >= 14
- npm packages: `ws`, `net`

**Client (ReactJS):**
- Node.js >= 14
- Vite
- React 18+

---

## 🎯 Luồng chơi mẫu

### Mode 0 (Classic):
1. Tạo phòng mode 0 → Tự động vào phòng
2. Bấm Start → Nhận câu hỏi 1
3. Trả lời đúng → Nhận 200 điểm, câu 2
4. Dùng lifeline nếu khó
5. Trả lời sai câu 7 → Về mốc 5 = 2,000 điểm
6. Game over

### Mode 1 (Coop Mode):
1. Tạo phòng mode 1 → Đợi bạn bè (2-4 người)
2. Host bấm Start → Câu hỏi 1 hiện ra
3. Alice biết đáp án, bấm trả lời ĐÚNG → Cả team lên câu 2
4. Câu 5 khó, Bob dùng quyền "50:50" (Quyền này bị khóa với những người còn lại)
5. Câu 10, Carol bấm trả lời SAI → **GAME OVER** (Cả team thua)
6. (Nếu qua được câu 15 → Cả team cùng thắng 🎉)

### Mode 2 (Speed Attack):
1. Tạo phòng mode 2 → Đợi 2-3 người join
2. Host bấm Start → Câu hỏi 1
3. Alice trả lời sau 3s (đúng) → Chờ Bob
4. Bob trả lời sau 10s (đúng) → Alice +270, Bob +200
5. Next câu 2
6. Sau 15 câu → Điểm cao nhất thắng

---

## 🐛 Troubleshooting

**Lỗi biên dịch server:**
```bash
# Kiểm tra sqlite3 đã cài chưa
sqlite3 --version

# MacOS
brew install sqlite3

# Ubuntu/Debian
sudo apt-get install libsqlite3-dev
```

**Middleware không kết nối:**
- Kiểm tra server đã chạy chưa (`./game_app`)
- Kiểm tra port 8080 và 4000

**Client không load:**
- Kiểm tra middleware đã chạy chưa
- Clear cache browser (Ctrl+Shift+R)
- Check Console (F12) xem lỗi

---

## 📞 Liên hệ & Đóng góp
- Báo lỗi: Tạo issue hoặc liên hệ maintainer
- Đóng góp: Fork repo và tạo Pull Request

---

**Chúc bạn chơi vui vẻ! 🎉**
