# Hướng Dẫn Chạy Dự Án

## Cấu Trúc
- **Database (`database/`)**: Data SQLite.
- **Server (`server/`)**: Backend logic (C).
- **Middleware (`middleware/`)**: Cầu nối WebSocket-TCP (NodeJS).
- **Client (`client/`)**: Frontend (ReactJS).

## Cách Chạy
Mở 3 terminal riêng biệt:

1.  **Server**:
    ```bash
    cd server/src
    make
    ./game_app
    ```
    *(Chạy port 8080)*

2.  **Middleware**:
    ```bash
    cd middleware
    npm install  # Lần đầu
    npm start
    ```
    *(Chạy port 4000)*

3.  **Client**:
    ```bash
    cd client
    npm install  # Lần đầu
    npm run dev
    ```
    *(Truy cập link localhost hiển thị)*

## Lưu Ý Quan Trọng
- **Logs**: File log trong `logs/` được sinh tự động. **KHÔNG tạo thủ công** để tránh lỗi mã hóa.
- **Database**: File `database/database.db` chứa dữ liệu game.
