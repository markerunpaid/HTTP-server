#include <iostream>
#include <cstring>
#include <unistd.h>     //close fd
#include <sys/socket.h> //all the important socket functions
#include <netinet/in.h> //ipv4
#include <arpa/inet.h>  //converts ip add text to binary
#include <array>

int main()
{

    // 1. create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        return 1;
    }

    // 2. create address
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);       // host to network small(16 bits)
    address.sin_addr.s_addr = INADDR_ANY; // listen on all the ip addresses of this device

    // 3. bind the socket to the address
    if (bind(server_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)))
    {
        perror("bind");
        return 1;
    }
    // reinterpret_cast is fast, it simply treats the value stored at that address as the new type argument

    // 4. start listening
    if (listen(server_fd, 10) == -1)
    {
        perror("listen");
        return 1;
    }

    std::cout << "Listening on port " << ntohs(address.sin_port) << '\n';

    // 5. accept a connection blocking
    // we dont need the client address info, so we dont make sockaddrin client, and pass it into accept, if we want we can do so and accept will set the client add in the sockaddrin object
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd == -1)
    {
        perror("accept");
        return 1;
    }
    std::cout << "Client connected\n";

    // 6. copy the data from kernel buffer to process memory, blocking
    std::array<char, 1025> buffer{};
    ssize_t bytes_recieved = recv(client_fd, buffer.data(), 1024, 0);

    if (bytes_recieved == -1)
    {
        perror("recv");
        // if we set recv to be non blocking and the client is connected but idle, then recv would return -1 and set errno so we can figure out what's exactly the case
    }
    else if (bytes_recieved == 0)
    {
        std::cout << "Client closed the connection\n";
        close(server_fd);
    }
    else
    {
        buffer[bytes_recieved] = '\0'; // add a null terminator
        // print the recieved data
        std::cout << buffer.data() << '\n';

        // 7. send message
        // when blocking - blocks if the kernel's send buffer is full
        // when non-blocking - returns -1 and sets errno to EAGAIN or EAWOULDBLOCK

        const char *message = "Hello client";
        // remember from learncpp, char message[] = "", would end up creating 2 copies of the string literal, first in the data section, because literal is static const, then assign the memory to message array and then copy the literal into that memory

        if (send(client_fd, message, strlen(message), 0) == -1)
        {
            perror("send");
            return 1;
            // similarily here also, you can get more information about the error from the errno
        }
    }

    close(client_fd);
    close(server_fd);

    return 0;
}