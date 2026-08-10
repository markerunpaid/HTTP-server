#include <string>
#include <unordered_map>

class HTTPresponse
{
public:
    enum class HTTPStatus
    {
        OK = 200,
        CREATED = 201,
        BAD_REQUEST = 400,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500
    };

private:
    HTTPStatus status_code_{}; // status code will be mapped to phrase later during serialization
    std::unordered_map<std::string, std::string> headers_;
    std::string body_{};
    std::string phrase(HTTPStatus status) const
    {
        switch (status)
        {
        case HTTPStatus::OK:
            return "OK";
        case HTTPStatus::CREATED:
            return "CREATED";
        case HTTPStatus::BAD_REQUEST:
            return "BAD REQUEST";
        case HTTPStatus::NOT_FOUND:
            return "NOT FOUND";
        case HTTPStatus::INTERNAL_SERVER_ERROR:
            return "INTERNAL SERVER ERROR";
        }
    }

public:
    std::string serialize() const
    {
        // response line
        std::string result{};
        result.reserve(512 + body_.size());

        result += "HTTP/1.1 ";
        result += std::to_string(static_cast<int>(status_code_));
        result += " ";
        result += phrase(status_code_);
        result += "\r\n";

        // headers
        for (const auto &[key, value] : headers_)
        {
            result += key;
            result += ": ";
            result += value;
            result += "\r\n";
        }

        result += "Content-length: ";
        result += std::to_string(body_.length());
        result += "\r\n";

        // blank line
        result += "\r\n";

        // body
        result += body_;

        return result;
    }

    // setter functions
    void set_status(HTTPStatus status)
    {
        status_code_ = status;
    }

    void set_header(const std::string &key, const std::string &value) // move
    {
        headers_.emplace(key, value);
    }

    void set_body(std::string body) // pass by value move idiom
    {
        body_ = std::move(body);
    }
};