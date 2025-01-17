from http.server import HTTPServer, BaseHTTPRequestHandler
import json
from urllib.parse import urlparse, parse_qs
import socket

log_traker = {'message':[]}

class TrackerHTTPServer(BaseHTTPRequestHandler):
    registry = {}
    
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')  # Cho phép tất cả các origin
        self.send_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')
        self.send_header('Access-Control-Allow-Credentials', 'true') 
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def do_POST(self):
        if self.path == '/peer-update':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode())

            file_name = data['file_name']
            peer_ip = data['peer_ip']
            peer_port = data['peer_port']
            pieces_indices = data['pieces_indices']
            file_details = data.get('file_details', None)
            if file_name not in self.registry:
                self.registry[file_name] = {
                    "piece_indices": {},
                    "files_nested": []
                }
            for index in pieces_indices:
                if index not in self.registry[file_name]["piece_indices"]:
                    self.registry[file_name]["piece_indices"][index] = []
                if (peer_ip, peer_port) not in self.registry[file_name]["piece_indices"][index]:
                    self.registry[file_name]["piece_indices"][index].append((peer_ip, peer_port))
            if file_details:
                self.registry[file_name]['files_nested'] = file_details
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            response = {"message": "Update successful"}
            self.wfile.write(json.dumps(response).encode())
            log_traker['message'].append(self.registry)
        elif self.path == '/peer-update-download':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode())

            peer_ip = data['peer_ip']
            peer_port = data['peer_port']
            file_name = data['file_name']
            pieces_indices = data['pieces_indices']
            for index in pieces_indices:
                if (peer_ip, peer_port) not in self.registry[file_name]["piece_indices"][index]:
                    self.registry[file_name]["piece_indices"][index].append((peer_ip, peer_port))
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            response = {"message": "Update successful"}
            self.wfile.write(json.dumps(response).encode())
            log_traker['message'].append(self.registry)

    def do_GET(self):
        if self.path == '/show':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            temp = []
            for key in self.registry.keys():
                if (len(self.registry[key]['files_nested'])):
                    for each in self.registry[key]['files_nested']:
                        temp.append(each['name'])
                temp.append(key)
            self.wfile.write(json.dumps({'files': temp}, indent=4).encode('utf-8'))
        elif self.path.startswith('/get-peer'):
            query_components = parse_qs(urlparse(self.path).query)
            piece_indices = query_components.get('piece_indices', [''])[0]
            filename = query_components.get('filename', [''])[0]
            log_traker['message'].append('INDEX: ' + piece_indices)
            log_traker['message'].append('NAME: ' + filename)
            try:
                piece_indices = [int(index) for index in piece_indices.split(',')]
            except ValueError:
                self.send_error(400, "Invalid piece indices")
                return
            response_data = self.find_peers_by_piece_indices(filename, piece_indices)
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response_data).encode('utf-8'))
        elif self.path == '/log-tracker':
                self.send_response(200)
                self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')
                self.send_header('Access-Control-Allow-Credentials', 'true')  # Thêm dòng này
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps(log_traker).encode())
                self.wfile.flush()
                log_traker["message"].clear()
        else:
            self.send_error(404, "File Not Found")

    def find_peers_by_piece_indices(self, filename, piece_indices):
        file_data = self.registry.get(filename, {})
        pieces_info = file_data.get('piece_indices', {})
        result = {index: pieces_info.get(index, []) for index in piece_indices}
        return result


def run(server_class=HTTPServer, handler_class=TrackerHTTPServer, port=8000):
    host_name = socket.gethostname()
    ip = socket.gethostbyname(host_name)
    server_address = (ip, port)
    httpd = server_class(server_address, handler_class)
    log_traker['message'].append(f"Starting httpd server on port {port}")
    httpd.serve_forever()


if __name__ == "__main__":
    run()