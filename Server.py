import socket
import threading
import json
from http.server import HTTPServer, BaseHTTPRequestHandler

# Khởi tạo biến log_server
log_server = {'message': []}

class ActionHandler(BaseHTTPRequestHandler):
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')  # Cho phép tất cả các origin
        self.send_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')
        self.send_header('Access-Control-Allow-Credentials', 'true') 
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
            
    def do_GET(self):
        if self.path == '/log-server':
            self.send_response(200)
            self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')
            self.send_header('Access-Control-Allow-Credentials', 'true')
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(log_server).encode())
            self.wfile.flush()
            log_server["message"].clear()

class Server(threading.Thread):
    def __init__(self, port=8001):
        super().__init__()
        self.host_name = socket.gethostname()  # Lấy hostname của server hiện tại
        self.ip = socket.gethostbyname(self.host_name)
        self.port = port
        self.max_peers = 10 
        self.running = True
        self.torrents = []
        self.http_thread = threading.Thread(target=self.run_http_server)

    def run_http_server(self):
        http_server = HTTPServer(('127.0.0.1', self.port + 2), ActionHandler)
        print(f"HTTP server is listening on 127.0.0.1:{self.port + 2}")
        http_server.serve_forever()

    def run(self):
        self.http_thread.start()  # Bắt đầu luồng HTTP
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind((self.ip, self.port))  # 8000
        server_socket.listen(self.max_peers)
        print(f"Server is listening on {self.ip}:{self.port}")

        while self.running:
            client_socket, addr = server_socket.accept()
            print(f"Server {self.ip}:{self.port} connected to {addr}")
            self.handle_client(client_socket)

    def handle_client(self, client_socket):
        try:
            while self.running:
                data = client_socket.recv(1024).decode()
                if not data:
                    break
                parts = data.split()
                cmd = parts.pop()
                if cmd == 'add':
                    last_closing_brace_index = data.rfind('}')
                    json_str = data[:(last_closing_brace_index + 1)]
                    json_obj = json.loads(json_str)
                    self.torrents.append(json_obj)
                    msg = f"Added torrent:\n{json.dumps(json_obj, indent=4)}"
                    log_server["message"].append(msg)  # Ghi log
                    client_socket.sendall("Added".encode())
                elif cmd == 'get':
                    last_closing_brace_index = data.rfind('}')
                    json_str = data[:(last_closing_brace_index + 1)]
                    json_obj = json.loads(json_str)
                    for torrent in self.torrents:
                        if torrent['info']['name'] == json_obj["filename"]:
                            client_socket.sendall(json.dumps(torrent).encode())
                            break
                    else:
                        client_socket.sendall("File not found".encode())
        finally:
            client_socket.close()

    def stop(self):
        self.running = False
        temp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        temp_socket.connect((self.ip, self.port))
        temp_socket.close()

def main():
    server = Server()
    server.start()  # Bắt đầu server
    try:
        while True:
            pass  # Giữ cho server chạy mãi mãi
    except KeyboardInterrupt:
        server.stop()
        server.join()

if __name__ == "__main__":
    main()
