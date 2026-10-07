#include "HTTP_parser.hpp"

HTTPrequest::HTTPmethod HTTP_parser::parse_method(std::string_view method)
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

HTTPrequest HTTP_parser::parse(std::string_view src)
{

    // request line
    std::size_t line_end = src.find("\r\n");
    if (line_end == std::string::npos)
    {
        return HTTPrequest{HTTPrequest::HTTPmethod::UNKNOWN, "", "", {}, ""};
    }

    //* method path and version from the request line
    std::string_view req_line = src.substr(0, line_end);
    size_t m_end = src.find(' ');
    size_t p_end = src.find(' ', m_end + 1);
    if (m_end == std::string_view::npos || p_end == std::string_view::npos)
        return {HTTPrequest::HTTPmethod::UNKNOWN, "", "", {}};
    std::string_view m = src.substr(0, m_end);
    std::string_view path = src.substr(m_end + 1, p_end - m_end - 1);
    std::string_view version = src.substr(p_end + 1);

    HTTPrequest::HTTPmethod method{parse_method(m)};

    // headers
    std::vector<HTTPrequest::Header> headers;
    headers.reserve(16); //* to avoid copying while relocations

    size_t header_start = line_end + 2;
    while (header_start < src.length())
    {
        size_t next_rn = src.find("\r\n", header_start);
        if (next_rn == std::string::npos)
        {
            break;
        }
        // last empty line, now body starts
        if (next_rn == header_start)
        {
            header_start += 2;
            break;
        }

        std::string_view header_line = src.substr(header_start, next_rn - header_start);
        size_t colon_pos = header_line.find(":");

        if (colon_pos != std::string::npos)
        { // avoid malformed header
            std::string_view key = header_line.substr(0, colon_pos);
            std::string_view value = header_line.substr(colon_pos + 1);

            // strip leading spaces
            while (!key.empty() && key.front() == ' ')
            {
                key.remove_prefix(1);
            }
            while (!key.empty() && key.back() == ' ')
            {
                key.remove_suffix(1);
            }
            while (!value.empty() && value.front() == ' ')
            {
                value.remove_prefix(1);
            }
            while (!value.empty() && value.back() == ' ')
            {
                value.remove_suffix(1);
            }

            if (!key.empty() && value.empty()) //* a copy will take place and it is intended as httprequest object will own the request
                headers.push_back({key, value});
        }
        header_start = next_rn + 2;
    }
    std::string_view body = src.substr(header_start);

    return HTTPrequest(method, path, version, std::move(headers), body); //* header would anyways be destroyed after the function call to parser returns, so better utilize that
}