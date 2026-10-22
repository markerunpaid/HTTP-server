#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <system_error>
#include <iostream>
#include <fcntl.h>
#include <cstddef>
#include <optional>

enum class IoStatus
{
    Ok,
    Blocked,
    Closed,
    Error
};

struct IoResult
{
    IoStatus status{};
    size_t bytes{};
    int errCode{};
};

class TCPsocket
{
    int fd_{-1};                         // underscore is naming convention to denote member variable
    explicit TCPsocket(int fd) noexcept; // socket is already created just set the fd_ with provided fd

public:
    TCPsocket();

    ~TCPsocket() noexcept; // destructors should never throw because it will lead to partial unbinding

    // delete copy constructor
    TCPsocket(const TCPsocket &src) = delete;

    // delete copy assignment
    TCPsocket &operator=(const TCPsocket &src) = delete;

    friend void swap(TCPsocket &a, TCPsocket &b) noexcept;

    // move constructor
    TCPsocket(TCPsocket &&src) noexcept;
    // move assignment, we take by value because we also want to transfer L values
    TCPsocket &operator=(TCPsocket src) noexcept;

    void bind(uint16_t port); // ports are 16 bit unsigned integer

    void listen(int backlog = 10);

    // send size number of bytes from buffer to member's fd
    IoResult send(const void *buffer, std::size_t size) noexcept;

    // recieve size number of bytes into buffer from member's fd
    IoResult recv(void *buffer, std::size_t size) noexcept;

    std::optional<TCPsocket> accept(int &err) noexcept;

    void set_reuse_addr(bool enable = true);

    void set_non_blocking(bool enable = true);

    int native_handle() const noexcept { return fd_; }
};