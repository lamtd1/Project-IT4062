import io from 'socket.io-client';

// Initialize socket connection using environment variable
const middlewareUrl = import.meta.env.VITE_MIDDLEWARE_URL || 'http://localhost:4000';
export const socket = io(middlewareUrl, {
    transports: ['websocket'],
    autoConnect: true
});

// const middlewareUrl = 'http://localhost:4000';
// export const socket = io(middlewareUrl, {
//     transports: ['websocket'],
//     autoConnect: true
// });