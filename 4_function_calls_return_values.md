### recv
0 = EOF has arrived from the client, no more further bytes would arrive

EINTR - operation was interrupted; check whether you should retry

EAGAIN/EWOULDBLOCK - socket not ready right now
- in send EAGAIN means the sender's out buffer is full


### connections errors
1. ECONNRESET
   - the client has disconnected/closed their socket, when the sender tries send/recv on that socket, this error is recieved
   - ACTION-
     - clean up the client socket
     - do not break the entire server for a single reset
     - log the reset
2. EPIPE
   - remote host sends FIN
   - if client still sends, receives RST (kernel marks the socket closed)
   - if client sends again, os triggers SIGPIPE signal and terminates the sending process(which is our server if we do not suppress or ignore SIGPIPE)
   - when supressed, the process is not terminated instead the system call send, returns -1 and sets errno to EPIPE

For both errors, the underlying TCP connection is completely dead. Your network layer should take identical cleanup steps:
1. Stop Writing/Reading: Immediately cease I/O operations on the socket descriptor.
2. Close Socket: Call close(sockfd) to release OS kernel resources.
3. Unregister Event Loop: Remove the socket from your event loop (epoll, kqueue, or select).
4. Clean Session State: Free memory buffers and notify application layers of the disconnection.

one naive way to deal with EAGAIN EWOULDBLOCK is to keep looping until the socket is ready to read/write again, but this is called busy waiting, instead the smarter approach is to put the socket into epoll (EPOLLIN/EPOLLOUT) and let epoll notify you when the socket is ready