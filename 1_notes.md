listen does the 3 way handshake, once completed it puts the connection into the queue, when accept is called the first connection is popped from the queue
Server

socket()
      │
bind()
      │
listen()
      │
accept()  ◄──────────────┐
      │                  │
      ▼                  │
recv()                   │
      │                  │
send()                   │
      │                  │
close(client_fd)         │
      │                  │
      └──────────────────┘
       Wait for next client

close frees the client socket, and on next accept it can be used again for some other client

0 - 1023 well known ports
1024 - 49151 registered ports
49152 - 65535 ephimeral ports (used by clients)
rest



listen - just puts the socket into listening state, now the kernel handles the incoming network packets
suppose server is blocked at accept and a new connection request comes
the 3 way handshake (syn, syn-ack, ack) is entirely handled by the kernel, the application never sees these packets, it only comes into play after accept
Network Card
      │
      ▼
Hardware interrupt
      │
      ▼
Kernel network stack
      │
      ▼
Listening socket found
      │
      ▼
TCP handshake
      │
      ▼
Connection added to accept queue
      │
      ▼
Wake up accept()

sszie_t = signed integer used in posix so that it can handle the negative error code returned


use non blocking calls - because with blocking we have to use 1000s of threads(1 for each client)
disable tcp_no delay - nagles (it stalls send untill all previous ack or buffer size is max seg size)
reuse address - time wait solved
reuse port - craziest, now we can use multiple different threads to listen in parallel, on the same port no, so accept is also parallelised, earlier we accepted connections serially and then handelled parallely, now they are accepted and handeled both parellely


### PHASE -2
make calls non-blocking
we have 2 choices
1. one thread per client(blocking calls block the respective thread)
- lot of threads as client increase
- context switching
- thread management
- storage wastage
2. event driven server(non blocking)
- 1 thread handelling a large no of clients
- multiple threads together handeling a very large no of clients


### PHASE - 3 (HTTPrequest and HTTP_parser zero allocation)
1. make copy free
- http request - use string views
- string_view vs const string& = when f("hi"), then string view simply takes the pointer, while const ref first constructs a temporary string object and takes reference to that
- no more istringstream - it does deep copy of the string, as it need a mutable buffer
2. use vector of Header instead of unordered map, more efficient


### 20-10-26 Per client buffer(we name this connection struct)
- right now the each client is just a socket in the map maintained by server
- partial reads and right need to maintain a different states for each client
- for this we create a struct for each client
- connection struct - TCPsocket object(it owns this, the socket is moved into this), buffer, how many bytes in that buffer is valid

### 21-10-26
- we need to identify if the request buffer holds a complete request at any moment
- For this, we create a simple struct, frame result, contains a bool (T/F) and the length of the request
- right now a request is read, when it's complete its processed and then the socket is discared, and a new socket is created for another request
- what if a client sends 2 request and they are small enough such that the 2nd request is read along with the 1st in the same buffer, if we discard after processing 1st req 2nd will get lost, we will fix this in next pipelining stage

### 22-10-26 minimal throwing send and recv
- a throw and catching it is much more costly than handeling the return value, moreover eagain/ewouldblock happen every now and then so we will remove throw from them in send/recv

### 22-10-26 make accept non-throwing

### 22-10-26 partial sends
How do we handle this

a client makes a request, we create a response

write it into conn.buff

and flush it

we write some of the response and then socket gets blocked

next time we recieve a request from the same client and we synthesise a response

now if we overwrite the wbuf then we will loose the last unsent response?
- When flush() hits Blocked, it re-registers the socket for EPOLLOUT only, we do not listen to any new request until the response is completely written

### how send works
- when we call send(fd, buffer, len, flags), os shifts form user mode to kernel mode, accquires the send_buff lock, and copies the bytes user space memory to the kernel buffer
- now tcp comes into play, and our work is done(application layer), we can safely assume that once the data is pushed into the kernel buffer, it would be sent to the the client on the other side(ack, retransmission, timouts etc are all handled by TCP)

