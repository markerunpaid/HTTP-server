#include "TCPsocket.hpp"
#include "Connection.hpp"
#include "EPOLL.hpp"
#include "HTTP_parser.hpp"
#include "HTTPresponse.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstring>
constexpr int MAX_EVENTS = 64;

int main()
{
    try
    {
        TCPsocket server;
        server.set_reuse_addr(true);
        server.bind(8080);
        server.listen(128);
        server.set_non_blocking(true);

        Epoll loop;
        loop.add_fd(server.native_handle(), EPOLLIN);

        std::cout << "Event-driven server running on http://localhost:8080...\n";

        // Store active client socket Connections
        std::unordered_map<int, Connection> clients;
        struct epoll_event events[MAX_EVENTS];

        while (true)
        {
            int num_events = loop.wait(events, MAX_EVENTS, -1);

            for (int i = 0; i < num_events; ++i)
            {
                int fd = events[i].data.fd;

                // 1. New incoming connection on listener socket
                if (fd == server.native_handle())
                {
                    while (true)
                    {

                        int e = 0;
                        auto client = server.accept(e);

                        if (!client)
                        {
                            // EAGAIN or EWOULDBLOCK means no more incoming connections
                            if (e == EAGAIN || e == EWOULDBLOCK)
                                break;
                            // try again
                            if (e == EINTR || e == ECONNABORTED)
                                continue;
                            std::cerr << "accept failed: " << std::strerror(e) << '\n';
                            break; // e.g. EMFILE: keep serving existing clients
                        }

                        //! rn
                        int small = 4096;
                        setsockopt(client->native_handle(), SOL_SOCKET, SO_SNDBUF, &small, sizeof(small));

                        int client_fd = client->native_handle();

                        loop.add_fd(client_fd, EPOLLIN);
                        clients.emplace(client_fd, std::move(*client));
                    }
                }
                // 2. client writable again
                else if (events[i].events & EPOLLOUT)
                {
                    //! client sanity check
                    auto it = clients.find(fd);
                    if (it == clients.end())
                        continue;

                    Connection &conn = it->second;

                    // still blocked
                    if (conn.flush() == FlushResult::Blocked)
                    {
                        continue;
                    }
                    // done or error
                    loop.del_fd(fd);
                    clients.erase(it);
                }
                // 3. Data ready to read on existing client socket
                else if (events[i].events & EPOLLIN)
                {

                    //! client sanity check
                    auto it = clients.find(fd);
                    if (it == clients.end())
                        continue;

                    Connection &conn = it->second;
                    TCPsocket &client = it->second.sock;

                    //! check if read buffer is full
                    if (conn.rlen == BUF_SIZE)
                    { // request buffer is full, but still no complete request(very long req, drop it)
                        loop.del_fd(fd);
                        clients.erase(it); // Client owns the socket, so erasing from here, delets that socket ==> closes the connection
                        continue;
                        // when the request size exceeds buffer, the server abruptly deletes the socket(closes it)
                        // when you close a connection which has data in it's recieved buffer, the kernel sends a RST to the client rather than a polite FIN
                    }

                    //! read request
                    IoResult r = client.recv(conn.rbuf + conn.rlen, BUF_SIZE - conn.rlen);
                    if (r.status == IoStatus::Blocked) // retry later
                        continue;
                    if (r.status != IoStatus::Ok) // Closed or Error
                    {
                        loop.del_fd(fd);
                        clients.erase(it);
                        continue;
                    }
                    // update the valid bytes length
                    conn.rlen += r.bytes;

                    //! check for complete request
                    FrameResult f_res = HTTP_parser::frame(conn.rbuf);
                    // incomplete do more reading
                    if (f_res.status == FrameStatus::Incomplete)
                    {
                        continue;
                    }

                    //! process complete request
                    std::string_view request_view(conn.rbuf, f_res.consumed);
                    HTTPrequest request = HTTP_parser::parse(request_view);

                    HTTPresponse response;
                    if (request.path() == "/" || request.path() == "/hello")
                    {
                        response.set_status(HTTPresponse::HTTPStatus::OK);
                        response.set_body("<html><body><h1>Hello from Epoll Async Server!</h1></body></html>");
                    }
                    else
                    {
                        response.set_status(HTTPresponse::HTTPStatus::NOT_FOUND);
                        response.set_body("<html><body><h1>404 Not Found</h1></body></html>");
                    }

                    std::string serialized_resp = response.serialize();

                    //! check if response exceed wbuff size
                    if (serialized_resp.size() > BUF_SIZE)
                    {
                        loop.del_fd(fd);
                        clients.erase(fd);
                        continue;
                    }

                    //! copy response into connection's wbuff
                    std::memcpy(conn.wbuf + conn.woff, serialized_resp.data(), serialized_resp.size());
                    conn.wlen += serialized_resp.size();
                    conn.woff = 0;

                    //! send response
                    switch (conn.flush())
                    {
                    case FlushResult::Blocked:
                        loop.mod_fd(fd, EPOLLOUT); // stop watching reads, wait until writable
                        break;
                    default: //!  close connection (only 1 request-response per connection)

                        loop.del_fd(fd);
                        clients.erase(it);
                        break;
                    }
                }
                // error events, if not sockets not deleted them, they would repeat again and again
                else if (events[i].events & (EPOLLERR | EPOLLHUP))
                {
                    auto it = clients.find(fd);
                    if (it != clients.end())
                    {
                        loop.del_fd(fd);
                        clients.erase(it);
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}