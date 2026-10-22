import socket, time
s = socket.socket()
s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024)
s.connect(("127.0.0.1", 8080))
s.sendall(b"GET / HTTP/1.1\r\nHost: x\r\n\r\n")
time.sleep(2)
data = b""
while True:
    chunk = s.recv(4096)
    if not chunk:
        break
    data += chunk
print(len(data), data.endswith(b"</html>"))