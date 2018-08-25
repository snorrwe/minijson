#pragma once
#include <stdexcept>
#include <string>

namespace MiniJson
{
struct ParseError : public std::exception
{
    std::string message;

    ParseError(std::string msg)
        : message(std::move(msg))
    {
    }
    ParseError(ParseError const&) = default;
    ParseError& operator=(ParseError const&) = default;

    const char* what() const override
    {
        return message.c_str();
    }
};

struct UnexpectedPropertyName : public ParseError
{
    UnexpectedPropertyName(std::string msg)
        : ParseError(std::move(msg))
    {
    }
    UnexpectedPropertyName(UnexpectedPropertyName const&) = default;
    UnexpectedPropertyName& operator=(UnexpectedPropertyName const&) = default;
};
} // namespace MiniJson

