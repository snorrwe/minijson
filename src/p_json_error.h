#pragma once
#include <stdexcept>
#include <string>

namespace mini_json
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

    virtual const char* what() const noexcept override
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
} // namespace mini_json

