#pragma once

#include "HTTPrequest.hpp"
#include <string>
#include <iostream>
#include <sstream>

class HTTP_parser
{
public:
    static HTTPrequest::HTTPmethod parse_method(const std::string &method)
    {
        if (method == "GET")
            return HTTPrequest::HTTPmethod::GET;

        if (method == "POST")
            return HTTPrequest::HTTPmethod::POST;

        if (method == "PUT")
            return HTTPrequest::HTTPmethod::PUT;

        if (method == "DELETE")
            return HTTPrequest::HTTPmethod::DELETE_;

        if (method == "PATCH")
            return HTTPrequest::HTTPmethod::PATCH;

        if (method == "HEAD")
            return HTTPrequest::HTTPmethod::HEAD;

        if (method == "OPTIONS")
            return HTTPrequest::HTTPmethod::OPTIONS;

        return HTTPrequest::HTTPmethod::UNKNOWN;
    }

    static HTTPrequest parse(const std::string &src)
    {
        std::istringstream stream(src);

        // request line
        std::string req_line{};
        getline(stream, req_line);

        // method path and version from the header line
        std::istringstream line_stream(req_line);
        std::string m, path, version;
        line_stream >> m >> path >> version;

        HTTPrequest::HTTPmethod method{parse_method(m)};

        // headers
        std::unordered_map<std::string, std::string> headers;
        std::string header;
        while (getline(stream, header))
        {
            if (header == "\r")
                break;
            int pos{header.find(':')};

            std::string key{header.substr(0, pos)};
            std::string value{header.substr(pos + 1)};

            // remove leading and trailing white-spaces
            while (!value.empty() && value.front() == ' ')
            {
                value.erase(0, 1);
            }
            while (!value.empty() && value.back() == ' ')
            {
                value.pop_back();
            }

            headers.emplace(key, value);
        }

        return HTTPrequest(method, path, version, headers);
    }
};