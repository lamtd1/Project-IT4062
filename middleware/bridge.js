import express from 'express'
import net from 'net'
import { createServer } from 'node:http'
import { Server } from 'socket.io'
import fs from 'fs'
import path from 'path'

// Hàm ghi lịch sử middleware
function log_middleware(message) {
    const now = new Date();
    const time_str = now.toISOString();
    const log_entry = `[${time_str}] ${message}\n`;
    const log_path = path.join(process.cwd(), '..', 'logs', 'middleware_history.txt');
    fs.appendFile(log_path, log_entry, (err) => {
        if (err) {
            console.error('Failed to write to log file:', err);
        }
    });
}

const app = express()
const server = createServer(app);
const io = new Server(server, {
    cors: {
        origin: "*",
        methods: ['GET', 'POST']
    }
})

const C_SERVER_PORT = 8080
const C_SERVER_HOST = '127.0.0.1'

io.on('connection', (webSocket) => {
    log_middleware(`React client connected: ${webSocket.id}`)

    const tcpSocket = new net.Socket();
    tcpSocket.connect(
        C_SERVER_PORT, C_SERVER_HOST, () => {
            log_middleware(`C server connected port: ${C_SERVER_PORT}`)
        }
    )
    // Handle chuyển dữ liệu client -> server
    webSocket.on('client_to_server', (data) => {
        tcpSocket.write(data)
    })

    // Handle chuyển dữ liệu server -> client
    // Parse messages using newline delimiter
    let buffer = Buffer.alloc(0);

    tcpSocket.on('data', (data) => {
        buffer = Buffer.concat([buffer, data]);

        // Split by newline delimiter
        let delimiterIndex;
        while ((delimiterIndex = buffer.indexOf('\n')) !== -1) {
            // Extract message (without delimiter)
            const message = buffer.slice(0, delimiterIndex);
            buffer = buffer.slice(delimiterIndex + 1);

            if (message.length > 0) {
                const opcode = message[0];
                const payload = message.slice(1);
                log_middleware("------ MESSAGE PARSED ------");
                log_middleware("Opcode: 0x" + opcode.toString(16).toUpperCase());
                log_middleware("Payload: " + payload.toString());
                log_middleware("----------------------------");

                // Emit to WebSocket
                webSocket.emit('server_to_client', message);
            }
        }
    });

    // Handle nếu phía server close
    tcpSocket.on('close', () => {
        log_middleware(`C Server closed connection for ${tcpSocket.id}`)
        webSocket.disconnect()
    })

    // Handle nếu phía client close
    webSocket.on('disconnect', () => {
        tcpSocket.end()
        log_middleware(`React Client disconnected: ${webSocket.id}`)
    })

    // Handle nếu có lỗi
    tcpSocket.on('error', (err) => {
        console.error(`Error: ${err.message}`);
    })

})

const PORT = 4000
server.listen(PORT, () => {
    log_middleware(`MIDDLEWARE RUNNING ON PORT: ${PORT}`)
})



