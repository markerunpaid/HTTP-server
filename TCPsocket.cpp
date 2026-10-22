#include "TCPsocket.hpp"

TCPsocket::TCPsocket(int fd) noexcept
    : fd_(fd)
{
}

TCPsocket::TCPsocket()
    : fd_{socket(AF_INET, SOCK_STREAM, 0)}
{
    if (fd_ == -1)
    {
        throw std::system_error(errno, std::system_category());
    }
}

TCPsocket::~TCPsocket() noexcept // destructors should never throw because it will lead to partial unbinding
{
    if (fd_ != -1)
        ::close(fd_);
}

void swap(TCPsocket &a, TCPsocket &b) noexcept
{
    using std::swap;
    swap(a.fd_, b.fd_);
}

TCPsocket::TCPsocket(TCPsocket &&src) noexcept
    : fd_(src.fd_) // we do not use traditional way of creating a new fd and then swapping, to avoid fd wastage
{
    src.fd_ = -1;
}

TCPsocket &TCPsocket::operator=(TCPsocket src) noexcept
{
    swap(*this, src);
    return *this;
}

void TCPsocket::bind(uint16_t port) // ports are 16 bit unsigned integer
{
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if (::bind(fd_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == -1)
    {
        throw std::system_error(errno, std::system_category(), "Failed to bind socket");
    }
}

void TCPsocket::listen(int backlog)
{
    if (::listen(fd_, backlog) == -1)
    {
        throw std::system_error(errno, std::system_category(), "Failed to listen");
    }
    std::cout << "Listening...\n";
}

std::optional<TCPsocket> TCPsocket::accept(int &err) noexcept
{
    // accept will create the new socket and return the file descriptor
    int client_fd = ::accept4(fd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC); // directly create non-blocking socket
    if (client_fd == -1)
    {
        err = errno;
        return std::nullopt;
    }
    err = 0;
    return TCPsocket(client_fd); // this will be moved into the client socket using the move constructor/assignment
}

IoResult TCPsocket::send(const void *buffer, std::size_t size)
{
    while (true)
    {
        ssize_t bytes_sent = ::send(fd_, static_cast<const char *>(buffer), size, MSG_NOSIGNAL);

        if (bytes_sent >= 0)
        {
            return {IoStatus::Ok, static_cast<size_t>(bytes_sent), 0};
        }
        else if (errno == EINTR)
        {
            continue;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return {IoStatus::Blocked, 0, errno};
        }
        else
            return {IoStatus::Error, 0, errno};
    }
}

IoResult TCPsocket::recv(void *buffer, std::size_t size)
{
    while (true)
    {
        ssize_t bytes_recieved = ::recv(fd_, static_cast<char *>(buffer), size, 0);

        if (bytes_recieved == 0)
        {
            return {IoStatus::Closed, 0, 0};
        }
        else if (bytes_recieved > 0)
        {
            return {IoStatus::Ok, static_cast<size_t>(bytes_recieved), 0};
        }
        else if (errno == EINTR)
        {
            continue;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return {IoStatus::Blocked, 0, errno};
        }
        else
        {
            return {IoStatus::Error, 0, errno};
        }
    }
}

void TCPsocket::set_reuse_addr(bool enable)
{
    int opt = enable ? 1 : 0;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Failed to set SO_REUSEADDR");
    }
}

void TCPsocket::set_non_blocking(bool enable)
{
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1)
    {
        throw std::system_error(errno, std::system_category(), "fcntl F_GETFL failed");
    }
    flags = enable ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    if (fcntl(fd_, F_SETFL, flags) == -1)
    {
        throw std::system_error(errno, std::system_category(), "fcntl F_SETFL failed");
    }
}