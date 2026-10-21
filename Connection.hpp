#include "TCPsocket.hpp"
#include <utility>                          //move
#include <cstddef>                          //size_t
constexpr std::size_t BUF_SIZE = 16 * 1024; // 16 kb

struct Connection
{
    TCPsocket sock{};
    char rbuf[BUF_SIZE]{}; // bytes recieved so far
    std::size_t rlen{0};   // no of bytes valid currently

    explicit Connection(TCPsocket &&src) // tcp sockets are non-copyable, only movable(so our fun only accepts r-value parameters)
        : sock(std::move(src))
    {
    }
};
