# Hướng Dẫn Chạy Dự Án

Tài liệu này hướng dẫn cách cài đặt và chạy toàn bộ hệ thống gồm 4 thành phần chính: Database, Server (Backend), Middleware, và Client (Frontend).

## 1. Tổng Quan Hệ Thống

Dự án bao gồm các thành phần:

*   **Database (`database/`)**: Chứa dữ liệu trò chơi (SQLite).
*   **Server (`server/`)**: Backend xử lý logic trò chơi, viết bằng C, giao tiếp qua TCP socket.
*   **Middleware (`middleware/`)**: Cầu nối viết bằng Node.js, chuyển đổi giữa WebSocket (Frontend) và TCP (Server).
*   **Client (`client/`)**: Giao diện người dùng viết bằng React (Vite).

## 2. Yêu Cầu Cài Đặt (Prerequisites)

Trước khi bắt đầu, hãy đảm bảo máy bạn đã cài đặt:

*   **GCC & Make**: Để biên dịch code C.
*   **Node.js & npm**: Để chạy Middleware và Client.
*   **SQLite3**: (Tùy chọn) Để quản lý database thủ công.

## 3. Các Bước Cài Đặt & Chạy

Bạn cần mở 3 terminal riêng biệt để chạy song song Server, Middleware và Client.

### Bước 1: Chuẩn Bị Database

Thư mục `database/` đã chứa sẵn file `database.db`. Nếu bạn muốn reset lại dữ liệu, hãy chạy lệnh sau:

```bash
cd database
rm database.db
sqlite3 database.db < schema.sql
sqlite3 database.db < data.sql
```

### Bước 2: Chạy Server (Backend)

Server viết bằng C chịu trách nhiệm xử lý logic chính.

1.  Mở terminal thứ nhất.
2.  Di chuyển vào thư mục source của server:
    ```bash
    cd server/src
    ```
3.  Biên dịch code (sử dụng Makefile):
    ```bash
    make
    ```
4.  Chạy server:
    ```bash
    ./game_app
    ```
    *Server sẽ lắng nghe tại port `8080`.*

### Bước 3: Chạy Middleware

Middleware đóng vai trò cầu nối, nhận request từ React (WebSocket) và gửi xuống Server C (TCP).

1.  Mở terminal thứ hai.
2.  Di chuyển vào thư mục middleware:
    ```bash
    cd middleware
    ```
3.  Cài đặt các thư viện cần thiết:
    ```bash
    npm install
    ```
4.  Khởi động middleware:
    ```bash
    npm start
    ```
    *Middleware sẽ chạy tại port `4000` (Websocket) và kết nối tới C Server tại `8080`.*

### Bước 4: Chạy Client (Frontend)

Giao diện người dùng để chơi game.

1.  Mở terminal thứ ba.
2.  Di chuyển vào thư mục client:
    ```bash
    cd client
    ```
3.  Cài đặt các thư viện:
    ```bash
    npm install
    ```
4.  Chạy ứng dụng ở chế độ development:
    ```bash
    npm run dev
    ```
    *Truy cập vào đường dẫn hiển thị trên terminal (thường là `http://localhost:5173`) để chơi.*

## 4. Giải Thích Chi Tiết

*   **Frontend (`client`)**: Là giao diện người dùng nơi người chơi tương tác. Nó kết nối tới Middleware thông qua giao thức `socket.io`.
*   **Middleware (`middleware`)**:
    *   Lắng nghe kết nối WebSocket từ Frontend.
    *   Khi nhận tin nhắn từ Frontend, nó chuyển tiếp (bridge) sang Server C qua TCP socket.
    *   Khi nhận phản hồi từ Server C, nó phân tích và gửi lại cho Frontend.
    *   File quan trọng: `bridge.js`.
*   **Backend (`server`)**:
    *   Chứa toàn bộ logic game: đăng nhập, đăng ký, tạo phòng, xử lý câu trả lời.
    *   Lưu trữ dữ liệu vào SQLite.
    *   File quan trọng: `src/main.c` (xử lý kết nối), `src/game.c` (logic game), `src/database.c` (truy vấn DB).
