#include <string>
#include <iostream>
#include <unordered_map>

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

    // move all the objects because we will access them through request object only
    HTTPrequest(HTTPmethod method, std::string path, std::string version, std::unordered_map<std::string, std::string> headers)
        : method_{method}, path_{std::move(path)}, version_{std::move(version)}, headers_{std::move(headers)}
    {
    }

    // getters
    HTTPmethod method() const
    {
        return method_;
    }

    const std::string path() const
    {
        return path_;
    }

    const std::string version() const
    {
        return version_;
    }

    const std::unordered_map<std::string, std::string> headers() const
    {
        return headers_;
    }

private:
    /* data */
    HTTPmethod method_{};
    std::string path_{};
    std::string version_{};
    std::unordered_map<std::string, std::string> headers_;
};
