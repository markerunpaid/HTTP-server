#include <string>
#include <iostream>
#include <unordered_map>
#include <string_view>
#include <vector>

class HTTPrequest
{

public:
    enum class HTTPmethod
    {
        GET,
        PUT,
        POST,
        OPTIONS,
        HEAD,
        DELETE_,
        PATCH,
        CONNECT,
        TRACE,
        UNKNOWN
    };

    /* data type*/
    struct Header
    {
        std::string_view key{};
        std::string_view value{};
    };

    //*all the string parameters converted to string view
    HTTPrequest(HTTPmethod method, std::string_view path, std::string_view version, std::vector<Header> headers, std::string_view body = "") // parser will pass r value for headers vector
        : method_{method}, path_{path}, version_{version}, headers_{std::move(headers)}, body_{body}
    {
    }

    // getters
    HTTPmethod method() const
    {
        return method_;
    }

    std::string_view path() const
    {
        return path_;
    }

    std::string_view version() const
    {
        return version_;
    }

    const std::vector<Header> &headers() const
    {
        return headers_;
    }

    std::string_view body() const
    {
        return body_;
    }

    // header lookup O(n)
    std::string_view get_header(std::string_view key) const
    {
        for (const auto &h : headers_)
        {
            if (h.key == key)
                return h.value;
        }
        return {};
    }

private:
    /* data */
    HTTPmethod method_{};
    std::string_view path_{};
    std::string_view version_{};
    std::vector<Header> headers_{};
    std::string_view body_{};
};
