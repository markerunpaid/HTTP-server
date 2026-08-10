#include "TCPsocket.hpp"
#include "HTTP_parser.hpp"
#include "HTTPresponse.hpp"
#include <iostream>

int main()
{
    try
    {
        TCPsocket server;
        server.set_reuse_addr(true);
        server.bind(8080);
        server.listen(128);

        std::cout << "Server starting on http://localhost:8080...\n";

        while (true)
        {
            TCPsocket client = server.accept();

            char buffer[4096]{};
            std::size_t bytes_received = client.receive(buffer, sizeof(buffer) - 1);
            if (bytes_received == 0)
                continue;

            std::string request_str(buffer, bytes_received);
            HTTPrequest request = HTTP_parser::parse(request_str);

            HTTPresponse response;
            std::cout<<"Client requested: "<<request.path()<<'\n';
            if (request.path() == "/" || request.path() == "/hello")
            {
                response.set_status(HTTPresponse::HTTPStatus::OK);
                response.set_body("<html><body><h1>Hello from Baseline C++ Server!</h1></body></html>");
            }
            else
            {
                response.set_status(HTTPresponse::HTTPStatus::NOT_FOUND);
                response.set_body("<html><body><h1>404 Not Found</h1></body></html>");
            }

            std::string serialized_resp = response.serialize();
            client.send(serialized_resp.data(), serialized_resp.size());
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}