import io from 'socket.io-client';

// Initialize socket connection (matches port in middleware)
export const socket = io('http://localhost:4000', {
    transports: ['websocket'],
    autoConnect: true
});
