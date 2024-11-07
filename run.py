import subprocess

# Danh sách các file Python cần chạy
files_to_run = ['Client2.py', 'Server.py', 'Tracker.py']

# Chạy từng file
processes = []
for file in files_to_run:
    process = subprocess.Popen(['python3', file])
    processes.append(process)

# Đợi tất cả các tiến trình kết thúc
for process in processes:
    process.wait()