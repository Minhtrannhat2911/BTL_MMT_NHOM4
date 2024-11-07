import hashlib
import threading
import socket
import os
import time
import requests
import math
import json
from datetime import datetime
import pprint
import random
import cgi
from contextlib import suppress
from http.server import  HTTPServer, BaseHTTPRequestHandler


log_client = {'message':[]}

# TRACKER_URL = 'http://192.168.0.112:8000'
TRACKER_URL = 'http://127.0.1.1:8000'

class File:
    def __init__(self, path: str, ip):
        self.piece_size = 102400
        self.block_size = self.piece_size // 2
        self.path = path
        self.peer_ip = ip

    def calculate_sha1(self, data):
        sha1_hash = hashlib.sha1()
        if isinstance(data, str):
            data = data.encode()
        sha1_hash.update(data)
        sha1_digest = sha1_hash.hexdigest()
        return sha1_digest # trả về chuỗi đã được mã hóa

    def divide_file_into_pieces(self):
        name = None
        if(self.file["name_folder"]):
            name = self.file["name_folder"]
        else:
            name = os.path.basename(self.file["files"][0]["disposition_options"]["filename"]) # thanhml.jpg
        pieces = []
        total_data = bytearray()
        file_info = {}
        piece_mappings = []
        current_offset = 0

        if self.file["name_folder"]: # Kiểm tra có phải là 1 thư mục không
            total_size = sum((file_size["filesize"])
                             for file_size in self.file["files"])
        elif self.file["files"][0]["disposition_options"]["name"] == '0': # Kiểm tra có phải file không
            total_size = self.file["files"][0]["filesize"] # Lấy file-size
        else:
            error_msg = "Provided path is neither a file nor a directory."
            log_client['message'].append(error_msg)
            raise ValueError(error_msg)

        if self.file.get("name_folder"):
            tree = {}
            for file in self.file["files"]:
                file_path = file["disposition_options"]["filename"]
                parts = file_path.split('/')
                parts.pop(0)
                current_level = tree

                for i, part in enumerate(parts):
                    if i == len(parts) - 1:
                        current_level[part] = None  # Đây là tệp cuối cùng
                    else:
                        current_level = current_level.setdefault(part, {})

            for file in self.file["files"]:
                file_path = file["disposition_options"]["filename"]
                # Ở đây, bạn có thể cần cung cấp một cách để xác định kích thước tệp nếu không sử dụng os
                file_size = file["filesize"]  # Giả sử bạn đã có thông tin kích thước tệp
                full_path = file_path.lstrip('/')  # Loại bỏ ký tự '/' ở đầu nếu có
                file_info[full_path] = file_size
                with open( "database/" + file_path, 'rb') as f:
                    file_data = f.read()
                    total_data.extend(file_data)
                    start_piece_index = current_offset // self.piece_size
                    end_piece_index = (current_offset + file_size - 1) // self.piece_size
                    piece_mappings.append({
                        'file_path': full_path,
                        'start_piece': start_piece_index,
                        'end_piece': end_piece_index,
                        'start_offset': current_offset,
                        'end_offset': (current_offset + file_size - 1)
                    })
                    current_offset += file_size
                    self.show_progress(name, current_offset, total_size)
        elif self.file["files"][0]["disposition_options"]["name"] == '0':  # Nếu là file
            with open("database/" + self.file["files"][0]["disposition_options"]["filename"], 'rb') as f:
                file_data = f.read()
                total_data.extend(file_data)
                full_path = name
                file_info[full_path] = total_size
                self.show_progress(name, total_size, total_size)

        for i in range(0, len(total_data), self.piece_size):
            piece = total_data[i:i + self.piece_size]
            pieces.append(piece)

        return {
            'name': name,
            'pieces': pieces,
            'info': {
                'file_info': file_info,
                'piece_mappings': piece_mappings
            }
        }

    def show_progress(self, filename, processed, total):
        progress = int(50 * processed / total) #50
        progress_bar = '#' * progress + '-' * (50 - progress) # '##################################################'
        # print(f"\rPeer {self.peer_ip}~{filename} {progress_bar} {int(100 * processed / total)}%", end='')
        msg = f"\rPeer {self.peer_ip}~{filename} \n{progress_bar} {int(100 * processed / total)}%"
        # '\rPeer 127.0.1.1~thanhml.jpg \n################################################## 100%'
        log_client['message'].append(msg)
        if processed >= total:
            print()
        time.sleep(0.5)

    def create_torrent_file(self, file_data):
        pieces_hash = ''.join([self.calculate_sha1(piece) for piece in file_data['pieces']]) # tạo hash cho từng piece và nối lại thành hash pieces
        torrent_data = {
            'announce': TRACKER_URL, # 'http://127.0.1.1:8000'
            'info': {
                'piece length': self.piece_size, # 102400
                'pieces': pieces_hash # mã băm
            }
        }
        if 'piece_mappings' in file_data['info'] and len(file_data['info']['piece_mappings']) > 0: 
            # nếu piece_mappings có tồn tại trong thuộc tính info của file_data và mảng piece_mappings đó lớn hơn 0
            torrent_data['info']['files'] = []
            torrent_data['info']['name'] = file_data['name']
            for mapping in file_data['info']['piece_mappings']:
                file_length = file_data['info']['file_info'].get(mapping['file_path'])
                if file_length is None:
                    error_msg = f"File size missing for {mapping['file_path']}"
                    log_client['message'].append(error_msg)
                    raise ValueError(f"File size missing for {mapping['file_path']}")
                file_entry = {
                    'length': file_length,
                    'path': mapping['file_path'].split(os.sep),
                    'mapping': {
                        'start_piece': mapping['start_piece'],
                        'end_piece': mapping['end_piece'],
                        'start_offset': mapping['start_offset'],
                        'end_offset': mapping['end_offset']
                    }
                }
                torrent_data['info']['files'].append(file_entry)
        else:
            single_file_key = next(iter(file_data['info']['file_info'])) # thamhml.jpg
            torrent_data['info']['name'] = single_file_key
            torrent_data['info']['length'] = file_data['info']['file_info'][single_file_key]
        return torrent_data

class ActionHandler(BaseHTTPRequestHandler):
    
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')  # Cho phép tất cả các origin
        self.send_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')
        self.send_header('Access-Control-Allow-Credentials', 'true') 
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
    
    def do_POST(self):
        if(self.path == '/upload'):
            ctype, pdict = cgi.parse_header(self.headers['Content-Type'])
            if ctype == 'multipart/form-data':
                # Đọc các dữ liệu từ body
                form = cgi.FieldStorage(
                    fp=self.rfile,
                    headers=self.headers,
                    environ={'REQUEST_METHOD': 'POST'}
                )
                # Lấy tệp từ FieldStorage
                data = {}
                if form.list != None:
                    list_file = form.list
                    data["files"] = []
                    data["cmd"] = 'upload'
                    data["name_folder"] = ""
                    if(len(list_file[0].filename.split("/", 1)) > 1):
                        data["name_folder"] = list_file[0].filename.split("/", 1)[0]
                    else:
                        data["name_folder"] = None
                    for file_item in list_file:
                            # Lưu file vào thư mục
                            file_path = "database/" + file_item.filename
                            if(data['name_folder'] != None):
                                file_path = os.path.join("database/" + data["name_folder"], file_item.filename.split("/", 1)[1])
                                # Tạo thư mục nếu nó chưa tồn tại
                                os.makedirs(os.path.dirname(file_path), exist_ok=True)
                            with open(file_path, 'wb') as f:  # Mở file với chế độ ghi nhị phân
                                f.write(file_item.file.read())
                            file_data = {
                                "disposition_options": file_item.disposition_options,
                                "filesize": file_item.bytes_read,
                            }
                            data['files'].append(file_data)
                    peer_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    current_port = self.server.server_address[1] - 1  # Kiểm tra cổng hiện tại
                    log_client['message'].append(f"Connecting to peer at {self.server.server_address[0]}:{current_port}")
        
                    try:
                        peer_socket.connect((self.server.server_address[0], current_port))
                        peer_socket.sendall(json.dumps(data).encode())
                        response = peer_socket.recv(1024)
                    except ConnectionRefusedError as e:
                        log_client['message'].append(f"Connection refused: {e}")
                    finally:
                        self.send_response(200)
                        self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')
                        self.send_header('Access-Control-Allow-Credentials', 'true')  # Thêm dòng này
                        self.send_header('Content-Type', 'application/json')
                        self.end_headers()
                        self.wfile.write(json.dumps(log_client).encode())
                        log_client['message'].clear()
                        peer_socket.close()
                            
                else:
                    self.send_response(400)
                    self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')
                    self.send_header('Access-Control-Allow-Credentials', 'true')  # Thêm dòng này
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    self.wfile.write(json.dumps(log_client).encode())
                    log_client['message'].clear()
                    
        elif self.path == '/download':
            # Đọc dữ liệu từ body của yêu cầu
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            try:
                request_data = json.loads(post_data)
                file_name = request_data.get('file_path')
                if file_name:
                    data = {}
                    data["cmd"] = 'download'
                    data["name_folder"] = file_name
                    # Gọi hàm để tải file từ peer
                    peer_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    current_port = self.server.server_address[1] - 1  # Kiểm tra cổng hiện tại
                    log_client['message'].append(f"Connecting to peer at {self.server.server_address[0]}:{current_port}")
                    peer_socket.connect((self.server.server_address[0], current_port))
                    peer_socket.sendall(json.dumps(data).encode())
                    response = peer_socket.recv(1024)
                    peer_socket.close()
                    
                    # Phản hồi thành công
                    self.send_response(200)
                    self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')
                    self.send_header('Access-Control-Allow-Credentials', 'true')  # Thêm dòng này
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    self.wfile.write(json.dumps(log_client).encode())
                    log_client['message'].clear()
                else:
                    # Nếu không có file_name, phản hồi lỗi
                    self.send_response(400)
                    self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')
                    self.send_header('Access-Control-Allow-Credentials', 'true')  # Thêm dòng này
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    self.wfile.write(json.dumps(log_client).encode())
                    log_client['message'].clear()
            except json.JSONDecodeError:
                self.send_response(400)
                self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')
                self.send_header('Access-Control-Allow-Credentials', 'true')  # Thêm dòng này
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps(log_client).encode())
                log_client['message'].clear()
            
    def do_GET(self):
        if self.path == '/all-files':
            folder_path = 'database'
            if os.path.exists(folder_path):
                result = self.get_files_and_folders(folder_path)
                self.send_response(200)
                self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')
                self.send_header('Access-Control-Allow-Credentials', 'true')  # Thêm dòng này
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps(result).encode())
            else:
                self.send_response(404)
                self.send_header('Access-Control-Allow-Origin', 'http://localhost:3000')
                self.send_header('Access-Control-Allow-Credentials', 'true')  # Thêm dòng này
                self.end_headers()
                self.wfile.write(b'Folder not found')
                
    def get_files_and_folders(self, path):
        items = []
        for entry in os.listdir(path):
            entry_path = os.path.join(path, entry)
            
            if os.path.isdir(entry_path):
                # Gọi đệ quy để lấy nội dung của thư mục
                contents = self.get_files_and_folders(entry_path)
                
                # Tính tổng kích thước của tất cả các tệp bên trong thư mục
                folder_size = sum(item['size'] for item in contents if 'size' in item)
                
                # Ngày cập nhật cuối cùng của thư mục dựa trên thời gian cập nhật mới nhất của các tệp con
                upload_date = max(
                    (datetime.fromisoformat(item['uploadDate']) for item in contents if 'uploadDate' in item),
                    default=datetime.fromtimestamp(os.path.getmtime(entry_path))
                ).isoformat()
                
                items.append({
                    'name': entry,
                    'type': 'directory',
                    'contents': contents,
                    'size': folder_size,
                    'uploadDate': upload_date
                })
            else:
                # Lấy kích thước và ngày tải lên cho tệp
                file_size = os.path.getsize(entry_path)
                upload_date = datetime.fromtimestamp(os.path.getmtime(entry_path)).isoformat()
                
                items.append({
                    'name': entry,
                    'type': 'file',
                    'size': file_size,
                    'uploadDate': upload_date
                })
        
        return items

class Peer(threading.Thread):
    def __init__(self, port=5005):
        super().__init__()
        self.host_name = socket.gethostname()
        self.peer_ip = '127.0.0.1'
        self.port = port
        self.server_socket = None
         # Khởi tạo HTTPServer trên một luồng khác
        self.http_thread = threading.Thread(target=self.run_http_server)
        self.server = HTTPServer(('127.0.0.1', port + 1), ActionHandler)
        self.running = True
        self.OUTPUT_PATH = os.path.join(os.getcwd(), 'output')
        self.files = []
        self.SERVER_IP = '127.0.1.1'  # CHANGE THIS TO YOUR SERVER IP
        self.SERVER_PORT = 8001  # THIS SHOULD MATCH THE PORT IN s.py
        self.handle_file = File('', self.peer_ip)
        
    def run_http_server(self):
        self.server.serve_forever()

    def run(self):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((self.peer_ip, self.port))
        self.server_socket.listen(10)
        self.http_thread.start() 
        # msg = f"\033[33mPeer {self.peer_ip}:{self.port} listening on port {self.port}\033[0m"
        msg = f"Peer {self.peer_ip}:{self.port} listening on port {self.port}"
        log_client['message'].append(msg)
        try:
            while self.running:
                client_socket, addr = self.server_socket.accept()
                if not self.running:
                    break
                # msg = f"\033[96mPeer {self.peer_ip}:{self.port} connected to {addr}\033[0m"
                smg = f"Peer {self.peer_ip}:{self.port} connected to {addr}"
                log_client['message'].append(smg)
                threading.Thread(target=self.handle_client, args=(client_socket,)).start()
        finally:
            # msg = f"\033[33mPeer {self.peer_ip} listening on port {self.port}\033[0m"
            msg = f"Peer {self.peer_ip} listening on port {self.port}"
            log_client['message'].append(msg)
            self.server.shutdown()
            self.server_socket.close()

    def update_tracker_upload(self, torrent_data):
        piece_length = torrent_data['info']['piece length'] # 102400
        if 'length' in torrent_data['info']:
            file_length = torrent_data['info']['length'] # 211848
            file_details = None
        else:
            file_length = sum(file['length'] for file in torrent_data['info']['files'])
            file_details = [{'name': "/".join(f['path']), 'length': f['length']} for f in torrent_data['info']['files']]
        number_of_pieces = math.ceil(file_length / piece_length) # 3
        payload = {
            "peer_ip": self.peer_ip, # '127.0.1.1'
            "peer_port": self.port, # 5005
            "file_name": torrent_data['info']['name'], # 'thanhml.jpg'
            "pieces_indices": list(range(number_of_pieces)), # [0,1,2]
            "file_details": file_details # 'add'
        }
        response = requests.post(torrent_data['announce'] + '/peer-update', json=payload) # req tới http://127.0.1.1:8000
        msg = f"Peer {self.peer_ip}:{self.port} " + response.text
        log_client['message'].append(msg)

    def update_tracker_download(self, torrent_data):
        payload = {
            "peer_ip": self.peer_ip,
            "peer_port": self.port,
            "file_name": torrent_data['file_name'],
            "pieces_indices": torrent_data['pieces_indices'],
        }
        response = requests.post(TRACKER_URL + '/peer-update-download', json=payload)
        msg = f"Peer {self.peer_ip}:{self.port} " + response.text
        log_client['message'].append(msg)

    def calculate_piece_indices_for_file(self, torrent_data, filename):
        piece_length = torrent_data['info']['piece length'] 
        files = torrent_data['info'].get('files', [])
        total_length = 0
        file_byte_ranges = {}
        for file in files:
            file_path = '/'.join(file['path']) #api/user.http
            start_byte = total_length # 0
            end_byte = start_byte + file['length'] - 1 #1192
            file_byte_ranges[file_path] = (start_byte, end_byte)
            total_length += file['length']
        if filename not in file_byte_ranges:
            if filename == torrent_data['info']['name']:
                total_length = torrent_data['info']['length'] if 'length' in torrent_data['info'] else total_length
                start_index = 0
                end_index = (total_length - 1) // piece_length
            else:
                return []
        else:
            start_byte, end_byte = file_byte_ranges[filename]
            start_index = start_byte // piece_length
            end_index = end_byte // piece_length
        return list(range(start_index, end_index + 1))

    def get_peers_for_pieces(self, tracker_url, filename, piece_indices):
        piece_indices_str = ','.join(map(str, piece_indices))
        url = f"{tracker_url}/get-peer?filename={filename}&piece_indices={piece_indices_str}"
        try:
            response = requests.get(url)
            response.raise_for_status()
            peer_data = response.json()
            # msg = f"\033[34mReceived peer-set: \033[0m{peer_data}\033[0m"
            msg = f"Received peer-set: {peer_data}"
            log_client['message'].append(msg)
            return peer_data
        except requests.RequestException as e:
            msg = f"Failed to get peer data: {e}"
            log_client['message'].append(msg)
            return {}

    def handle_client(self, client_socket):
        with client_socket:
            while True:
                data = client_socket.recv(1024).decode()
                response = 'Response OK'
                if not data:
                    break
                # parts = data.split()
                data_object = json.loads(data)
                
                cmd = data_object["cmd"]
                if (cmd == 'download'):
                    filename = data_object['name_folder']
                    torrent_data = self.get_torrent(filename)
                    requested_pieces = self.calculate_piece_indices_for_file(torrent_data, filename)
                    peer_set = self.get_peers_for_pieces(torrent_data['announce'], filename, requested_pieces)
                    info = {filename: {}}
                    is_success = [True]
                    threads = []
                    for piece_index, peer_ips in peer_set.items():
                        thread = threading.Thread(target=self.request_piece_from_peer,
                                                  args=(piece_index, peer_ips, filename, info, is_success))
                        thread.start()
                        threads.append(thread)
                    for thread in threads:
                        thread.join()
                    if is_success[0]:
                        temp = dict(sorted(info[filename].items()))
                        temp_hash = ''
                        for index in temp:
                            temp_hash += self.handle_file.calculate_sha1(temp[index])
                        msg = f"Downloaded pieces hash: {temp_hash}"
                        log_client['message'].append(msg)
                        if temp_hash == torrent_data['info']['pieces']:
                            msg = "Downloaded pieces match the hash in the torrent file."
                            log_client['message'].append(msg)
                            self.files.append(info)
                            data_update = {
                                "file_name": filename,
                                "pieces_indices": requested_pieces
                            }
                            self.update_tracker_download(data_update)
                            self.reconstruct_file(filename, os.walk(filename), torrent_data)
                            client_socket.sendall(response.encode())
                            msg = f"Peer {self.peer_ip}:{self.port} has downloaded: {filename}"
                            log_client['message'].append(msg)
                        else:
                            msg = f"Downloaded pieces do not match the hash in the torrent file."
                            log_client['message'].append(msg)

                            response = 'Response Failed'
                            client_socket.sendall(response.encode())
                    else:
                        response = 'Response Failed'
                        msg = f"Failed to download pieces, there seems to be an issue with the peer."
                        log_client['message'].append(msg)
                        client_socket.sendall(response.encode())
                elif (cmd == 'upload'):
                    files = data_object["files"]
                    folder = {
                        "files": files,
                        "name_folder": data_object["name_folder"]
                    }
                    self.handle_file.file = folder # /home/joyboy/thanhml.jpg
                    res = self.handle_file.divide_file_into_pieces()
                    self.files.append({res['name']: {str(i): value for i, value in enumerate(res['pieces'])}}) 
                    # 'thanhml.jpg' : {'0' : piece0, '1' : piece1,...}
                    for each in self.files:
                        for key, value in each.items(): # lấy key và value của từng file đã upload
                            for k, v in value.items():
                                msg = f"Piece {k} of file {key} has length: {len(v)}"
                                log_client['message'].append(msg)
                    torrent_data = self.handle_file.create_torrent_file(res)
                    self.update_tracker_upload(torrent_data)
                    json_str = json.dumps(torrent_data)
                    self.update_torrent_server(f"{json_str} add")
                    client_socket.sendall(response.encode())
                    if(len(folder['files']) > 1):
                        msg = f"Peer {self.peer_ip}:{self.port} has uploaded: {folder['name_folder']}"
                    else:
                        msg = f"Peer {self.peer_ip}:{self.port} has uploaded: {files[0]['disposition_options']['filename']}"
                        
                    log_client['message'].append(msg)
                elif (cmd == 'block'):
                    filename = data_object["file"]
                    index = data_object["piece_index"]
                    offset = data_object["block_offset"]
                    response = bytearray()
                    for each in self.files:
                        if filename in each:
                            if index in each[filename]:
                                piece = each[filename].get(index)
                                piece_length = len(piece)
                                offset = int(offset)
                                if (offset < piece_length):
                                    end = min(offset + self.handle_file.block_size, piece_length)
                                    response = piece[offset:end]
                                break
                            else:
                                error_msg = f"Piece {index} not found for file {filename}"
                                log_client['message'].append(msg)
                                raise ValueError(f"Piece {index} not found for file {filename}")
                    client_socket.sendall(response)
                elif (cmd == 'length'):
                    filename = data_object["file"]
                    index = data_object["piece_index"]
                    piece_length = 0
                    for each in self.files:
                        if filename in each:
                            piece = each[filename].get(index)
                            piece_length = len(piece)
                            break
                    client_socket.sendall(str(piece_length).encode())
                elif (cmd == 'construct'):
                    self.reconstruct_file(filename)
                    client_socket.sendall('Response OK'.encode())

    def stop(self):
        self.running = False
        temp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        temp_socket.connect((self.peer_ip, self.port))
        temp_socket.close()

    def get_torrent(self, filename):
        log_client['message'].append(f"{filename}")
        peer_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        peer_socket.connect((self.SERVER_IP, self.SERVER_PORT))
        jsonData = json.dumps({"filename":filename})
        peer_socket.sendall(f"{jsonData} get".encode())
        response = peer_socket.recv(1024).decode()
        peer_socket.close()

        torrent = json.loads(response)
        msg = f"Peer {self.peer_ip}:{self.port} has received torrent file:"
        log_client['message'].append(msg)
        log_client['message'].append(torrent)
        return torrent

    def request_block_from_peer(self, piece_index, block_offset, peer_ips, file, index, blocks, info):
        temp = peer_ips

        while True:
            value = random.choice(temp)
            try:
                peer_ip, peer_port = value
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect((peer_ip, peer_port))
                res = {
                    "piece_index": piece_index,
                    "block_offset": block_offset,
                    "file": file,
                    "cmd": 'block'
                }
                sock.sendall(json.dumps(res).encode())
                response = sock.recv(self.handle_file.block_size)
                sock.close()
                blocks[index] = response
                break
            except:
                with suppress(ValueError):
                    temp.remove(value)
                if len(temp) == 0:
                    info['is_success'] = False
                    break

    def request_piece_from_peer(self, piece_index, peer_ips, file, piece_info, is_success):
        piece_size = 0
        temp = peer_ips
        while True:
            value = random.choice(temp)
            try:
                peer_ip, peer_port = value
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect((peer_ip, peer_port))
                res = {
                    "file":file,
                    "piece_index":piece_index,
                    "cmd":"length",
                }
                sock.sendall(json.dumps(res).encode())
                piece_size = sock.recv(1024)
                sock.close()
                if (piece_size):
                    break
                else:
                    with suppress(ValueError):
                        temp.remove(value)
                    if len(temp) == 0:
                        return {'is_success': False}
            except:
                with suppress(ValueError):
                    temp.remove(value)
                if len(temp) == 0:
                    return {'is_success': False}
        piece = bytearray()
        blocks = {}
        block_offset = 0
        info = {'is_success': True}
        num = math.ceil(int(piece_size.decode()) / self.handle_file.block_size)
        pool = []
        for index in range(num):
            block_offset = index * self.handle_file.block_size
            thread = threading.Thread(target=self.request_block_from_peer,
                                      args=(piece_index, block_offset, peer_ips, file, index, blocks, info))
            thread.start()
            pool.append(thread)

        for thread in pool:
            thread.join()

        blocks = dict(sorted(blocks.items()))
        for index in blocks:
            piece.extend(blocks[index])
        info['piece'] = piece
        is_success[0] = info['is_success']
        piece_info[file][piece_index] = piece

    def reconstruct_file(self, target_filename, file_size, torrent_data):
        root = target_filename.split('/')[0]

        for file_dict in self.files:
            if root in file_dict:
                pieces = file_dict[root]
                sorted_piece_keys = sorted(pieces.keys(), key=int)
                complete_file_data = bytearray()
                for key in sorted_piece_keys:
                    complete_file_data.extend(pieces[key])
                if not os.path.exists(self.OUTPUT_PATH):
                    os.makedirs(self.OUTPUT_PATH)
                if ('length' in torrent_data['info']):
                    output_path = os.path.join(self.OUTPUT_PATH, root)
                    with open(output_path, 'wb') as file:
                        file.write(complete_file_data)
                    msg = f"File successfully reconstructed and saved to "
                    log_client['message'].append(msg)
                    msg = output_path
                    log_client['message'].append(msg)
                else:
                    name = torrent_data['info']['name']
                    files = torrent_data['info']['files']
                    for file_info in files:
                        file_path = os.path.join(*file_info['path'])
                        if target_filename == file_path:
                            dirs = file_info['path'][:-1]
                            output_dir = os.path.join(*dirs)
                            os.makedirs(output_dir, exist_ok=True)
                            output_path = os.path.join(self.OUTPUT_PATH, file_info['path'][-1])
                            with open(output_path, 'wb') as file:
                                start = file_info['mapping']['start_offset']
                                end = file_info['mapping']['end_offset']
                                file.write(complete_file_data[start:end])
                            msg = f"File successfully reconstructed and saved to "
                            log_client['message'].append(msg)
                            msg = output_path
                            log_client['message'].append(msg)
                            return
                    if target_filename == name:
                        for file_info in files:
                            dirs = file_info['path'][:-1]
                            output_dir = os.path.join(self.OUTPUT_PATH, *dirs)
                            os.makedirs(output_dir, exist_ok=True)
                            output_path = os.path.join(output_dir, file_info['path'][-1])
                            with open(output_path, 'wb') as file:
                                start = file_info['mapping']['start_offset']
                                end = file_info['mapping']['end_offset']
                                file.write(complete_file_data[start:end])
                        msg = f"File successfully reconstructed and saved to "
                        log_client['message'].append(msg)
                        msg = target_filename
                        log_client['message'].append(msg)

                return
        msg = f"File {target_filename} not found in the provided data."
        log_client['message'].append(msg)

    def update_torrent_server(self, data):
        peer_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        peer_socket.connect((self.SERVER_IP, self.SERVER_PORT))
        peer_socket.sendall(data.encode())
        response = peer_socket.recv(1024).decode()
        peer_socket.close()
if __name__ == "__main__":
    port = int(os.environ.get('REACT_PORT', 3000)) + 1  # Nhận cổng từ biến môi trường, mặc định là 3000
    peer = Peer(port=port)
    peer.start()
    try:
        while True:
            pass  # Giữ cho server chạy mãi mãi
    except KeyboardInterrupt:
        peer.stop()
        peer.join()
