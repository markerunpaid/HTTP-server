#include "TCPsocket.hpp"
#include "Connection.hpp"
#include "EPOLL.hpp"
#include "HTTP_parser.hpp"
#include "HTTPresponse.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>

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
                        try
                        {
                            TCPsocket client = server.accept();
                            client.set_non_blocking(true);
                            int client_fd = client.native_handle();

                            loop.add_fd(client_fd, EPOLLIN);
                            clients.emplace(client_fd, std::move(client));
                        }
                        catch (const std::system_error &e)
                        {
                            // EAGAIN or EWOULDBLOCK means no more incoming connections
                            if (e.code().value() == EAGAIN || e.code().value() == EWOULDBLOCK)
                                break;
                            throw;
                        }
                    }
                }
                // 2. Data ready to read on existing client socket
                else if (events[i].events & EPOLLIN)
                {
                    auto it = clients.find(fd);
                    if (it == clients.end())
                        continue;

                    Connection &conn = it->second;
                    TCPsocket &client = it->second.sock;

                    // request buffer is full, but still no complete request(very long req, drop it)
                    if (conn.rlen == BUF_SIZE)
                    {
                        loop.del_fd(fd);
                        clients.erase(it); // Client owns the socket, so erasing from here, delets that socket ==> closes the connection
                        continue;
                        // when the request size exceeds buffer, the server abruptly delets the socket(closes it)
                        // when you close a connection which has data in it's recieved buffer, the kernel sends a RST to the client rather than a polite FIN
                    }

                    // read bytes
                    std::size_t bytes_received = it->second.rlen;
                    try
                    {
                        bytes_received = client.receive(conn.rbuf + conn.rlen, BUF_SIZE - conn.rlen);
                    }
                    // handle error
                    catch (const std::system_error &e)
                    {
                        if (e.code().value() == EAGAIN || e.code().value() == EWOULDBLOCK)
                            continue;

                        // Client error or drop
                        loop.del_fd(fd);
                        clients.erase(it);
                        continue;
                    }
                    // no bytes recieved (Client closed connection)
                    if (bytes_received == 0)
                    {
                        loop.del_fd(fd);
                        clients.erase(it);
                        continue;
                    }

                    // update the valid bytes length
                    conn.rlen += bytes_received;

                    // check if complete request present
                    FrameResult f_res = HTTP_parser::frame(conn.rbuf);

                    // incomplete do more reading
                    if (f_res.status == FrameStatus::Incomplete)
                    {
                        continue;
                    }
                    // complete, thus process

                    // Process request
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
                    client.send(serialized_resp.data(), serialized_resp.size());

                    // Close connection after response (HTTP/1.1 short lived for now)
                    loop.del_fd(fd);
                    clients.erase(it);
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