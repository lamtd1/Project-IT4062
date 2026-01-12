# AI LÀ TRIỆU PHÚ - Game Trắc Nghiệm Multiplayer

## 🎯 Giới thiệu
Game **Ai Là Triệu Phú** trực tuyến multiplayer. Hỗ trợ 3 chế độ chơi với hệ thống phòng, điểm số và xếp hạng.

---

## � DÀNH CHO NGƯỜI CHƠI (Không cần cài đặt)

> **Chỉ cần mở link do người host chia sẻ** trong trình duyệt là có thể chơi được!
> 
> Ví dụ: `http://192.168.1.5:5173`

---

## 🖥️ DÀNH CHO NGƯỜI HOST (Chạy Server)

### Yêu cầu cài đặt
- **Node.js** >= 14
- **SQLite3**
- **GCC** compiler (có sẵn trên Mac/Linux)
- **Make**

### Bước 1: Khởi tạo Database (Lần đầu hoặc Reset)
```bash
cd database
rm -f database.db                    # Xóa DB cũ (nếu có)
sqlite3 database.db < schema.sql     # Tạo cấu trúc bảng
sqlite3 database.db < data.sql       # Import dữ liệu mẫu
cd ..
```

### Bước 2: Chạy Server (C)
```bash
cd server/src
make clean && make    # Compile
./game_app           # Chạy server (log tự động ghi vào logs/server_history.txt)
```

### Bước 3: Chạy Middleware (NodeJS)
Mở terminal mới:
```bash
cd middleware
npm install          # Lần đầu
npm start
```

### Bước 4: Chạy Client (ReactJS)
Mở terminal mới:
```bash
cd client
npm install          # Lần đầu
npm run dev -- --host    # QUAN TRỌNG: Thêm --host để cho máy khác truy cập
```

### Bước 5: Cấu hình IP cho LAN
1. Tìm IP máy host:
   - **MacOS:** `ipconfig getifaddr en0`
   - **Windows:** `ipconfig` (tìm IPv4 Address)
   - **Linux:** `hostname -I`

2. Sửa file `client/.env`:
```env
VITE_MIDDLEWARE_URL=http://[IP_MÁY_BẠN]:4000
```

3. Chia sẻ link cho bạn bè: `http://[IP_MÁY_BẠN]:5173`

---

## 📁 Cấu trúc dự án
```
Network/
├── database/           # SQLite database
├── logs/               # Log files (tự động tạo)
├── server/src/         # Backend C (Port 8080)
├── middleware/         # WebSocket bridge (Port 4000)
└── client/             # Frontend React (Port 5173)
```

---

## � Chế độ chơi

### Mode 0: Cổ Điển - Chơi đơn
- 👤 1 người chơi
- 💰 Thang điểm: 200 → 150,000
- 🛡️ Mốc an toàn: Câu 5 (2,000đ), Câu 10 (22,000đ)
- ❌ Sai = về mốc an toàn
- 🚶 Có thể Walk Away

### Mode 1: Hợp Tác - Đồng đội
- 👥 2-4 người (chung team)
- ✅ Ai đúng = cả team lên câu
- ❌ Ai sai = cả team thua
- ⏱️ Hết giờ = cả team thua
- 🆘 Quyền trợ giúp dùng chung

### Mode 2: Tốc Độ - Đua điểm
- 👥 2-4 người
- ⏳ Đợi tất cả trả lời mới next
- ⭐ Bonus điểm nếu trả lời nhanh
- ❌ Sai = không điểm (không loại)
- 🏆 Điểm cao nhất thắng

---

## 🎁 Quyền trợ giúp (Lifelines)
- 🔹 **50:50**: Loại 2 đáp án sai
- 🔹 **Khán giả**: % bình chọn
- � **Gọi điện**: 80% đúng
- 🔹 **Chuyên gia**: 100% đúng

---

## 🐛 Xử lý lỗi thường gặp

| Lỗi | Giải pháp |
|-----|-----------|
| Server không compile | Cài sqlite3: `brew install sqlite3` (Mac) |
| Middleware không kết nối | Kiểm tra server đã chạy (port 8080) |
| Client trắng | Kiểm tra middleware đã chạy (port 4000) |
| Máy khác không vào được | Kiểm tra firewall, chạy client với `--host` |

---

## 📊 Thang điểm Mode 0
```
Câu 1-5:   200, 400, 600, 1,000, 2,000 (Mốc an toàn)
Câu 6-10:  3,000, 6,000, 10,000, 14,000, 22,000 (Mốc an toàn)
Câu 11-15: 30,000, 40,000, 60,000, 85,000, 150,000
```

---

**Chúc các bạn chơi vui vẻ! 🎉**
