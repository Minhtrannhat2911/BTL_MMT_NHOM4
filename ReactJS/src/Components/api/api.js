import axios from "axios";

const reactPort = window.location.port;
const peerPort = Number(reactPort) + 2;

export const api = axios.create({
    baseURL: `http://localhost:${peerPort}/`,
    withCredentials: true,
})

// api.interceptors.request.use((config) => {
    
// })