#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <thread>

int main()
{
    int pipefd[2]{};
    pipe(pipefd);
    char msg[] = "hello there";

    // create a epoll instance
    int epollfd = -1;
    // info of each monitored fd
    struct epoll_event einf[10];
    // ready fds
    struct epoll_event eventqueue[10];

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        throw std::system_error(errno, std::system_category(), "epoll create failed");
    }

    // set event info
    einf[0].events = EPOLLIN;
    einf[0].data.fd = pipefd[0];

    // add fd to epoll set
    epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefd[0], &einf[0]);

    // writer thread
    std::thread writer([&]()
                       { 
        std::this_thread::sleep_for(std::chrono::seconds(5));

        const char msg[] = "Hello from the worker thread!";

        std::cout << "Writer thread: writing...\n";

        write(pipefd[1],msg, sizeof(msg));

        close(pipefd[1]); });

    std::cout << "Main thread: waiting in epoll_wait()...\n";

    // check if any event is ready
    int ready = epoll_wait(epollfd, eventqueue, std::size(eventqueue), -1);
    if (ready > 0)
    {
        for (int i = 0; i < ready; i++)
        {
            char buffer[100];

            ssize_t n =
                read(eventqueue[i].data.fd,
                     buffer,
                     sizeof(buffer));

            buffer[n] = '\0';

            std::cout << buffer << '\n';
        }
    }
}