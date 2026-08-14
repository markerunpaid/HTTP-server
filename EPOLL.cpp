#include "EPOLL.hpp"

Epoll::Epoll()
    : epfd_{epoll_create1(EPOLL_CLOEXEC)}
{
    if (epfd_ == -1)
    {
        throw std::system_error(errno, std::system_category(), "Failed to create epoll instance");
    }
}

Epoll::~Epoll() noexcept
{
    if (epfd_ != -1)
    {
        ::close(epfd_);
    }
}

// move const
Epoll::Epoll(Epoll &&src) noexcept
    : epfd_{src.epfd_}
{
    src.epfd_ = -1;
}

// move assgn
Epoll &Epoll::operator=(Epoll &&src) noexcept
{
    if (this == &src)
    {
        return *this;
    }

    // remove old descriptor
    if (epfd_ != -1)
    {
        ::close(epfd_);
    }

    // assign new
    epfd_ = src.epfd_;
    // hollow old object
    src.epfd_ = -1;

    return *this;
}

void Epoll::add_fd(int fd, uint32_t events)
{

    struct epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if (::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        throw std::system_error(errno, std::system_category(), "epoll_ctl ADD failed");
    }
}

void Epoll::mod_fd(int fd, uint32_t events)
{

    struct epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if (::epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1)
    {
        throw std::system_error(errno, std::system_category(), "epoll_ctl MOD failed");
    }
}

void Epoll::del_fd(int fd)
{

    if (::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1)
    {
        throw std::system_error(errno, std::system_category(), "epoll_ctl MOD failed");
    }
}

int Epoll::wait(struct epoll_event event_queue[], size_t max_events, int timeout)
{
    int ready_fds = epoll_wait(epfd_, event_queue, max_events, timeout);
    if (ready_fds == -1)
    {
        if (errno == EINTR)
        {
            return 0; // interrupted by signal
        }
        throw std::system_error(errno, std::system_category(), "epoll_wait failed");
    }
    return ready_fds;
}