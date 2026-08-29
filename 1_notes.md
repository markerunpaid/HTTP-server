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


use non blocking calls - because with blocking we have to use 1000s of threads
disable tcp_no delay - nagles (it stalls send untill all previous ack or buffer size is max seg size)
reuse address - time wait solved
reuse port - craziest, now we can use multiple different threads to listen in parallel, on the same port no, so accept is also parallelised, earlier we accepted connections serially and then handelled parallely, now they are accepted and handeled both parellely


PHASE -2
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


PHASE - 3
1. make copy free
- http request - use string views
- string_view vs const string& = when f("hi"), then string view simply takes the pointer, while const ref first constructs a temporary string object and takes reference to that
- no more istringstream - it does deep copy of the string, as it need a mutable buffer
2. use vector of Header instead of unordered map, more efficient

