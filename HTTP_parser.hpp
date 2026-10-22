#pragma once

#include "HTTPrequest.hpp"
#include <string>
#include <iostream>
#include <sstream>

enum class FrameStatus
{
    Complete,
    Incomplete
};

struct FrameResult
{
    FrameStatus status{};   // complete/incomplete
    std::size_t consumed{}; // length of request
};

class HTTP_parser
{
public:
    static HTTPrequest::HTTPmethod parse_method(std::string_view method);

    static HTTPrequest parse(std::string_view src);

    static FrameResult frame(std::string_view buf);
};