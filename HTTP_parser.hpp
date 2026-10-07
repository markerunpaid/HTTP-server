#pragma once

#include "HTTPrequest.hpp"
#include <string>
#include <iostream>
#include <sstream>

class HTTP_parser
{
public:
    static HTTPrequest::HTTPmethod parse_method(std::string_view method);

    static HTTPrequest parse(std::string_view src);
};