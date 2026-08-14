#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <system_error>

class Epoll
{
    int epfd_{-1};

public:
    Epoll();
    ~Epoll() noexcept;

    // delete copy const and assig
    Epoll(const Epoll &src) = delete;
    Epoll &operator=(const Epoll &src) = delete;

    // move const and assign
    Epoll(Epoll &&src) noexcept;
    Epoll &operator=(Epoll &&src) noexcept;

    void add_fd(int fd, uint32_t events);
    void mod_fd(int fd, uint32_t events);
    void del_fd(int fd);

    int wait(struct epoll_event event_queue[], size_t max_events, int timeout = -1);
};