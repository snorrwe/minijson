#pragma once
#include "p_json_error.h"
#include "p_json_utility.h"
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace mini_json::_private
{
template <typename FwIt> class ParseImpl
{
    enum class ParseState
    {
        Default,
        Key,
        Value
    };

    ParseState state = ParseState::Default;
    FwIt& begin;
    FwIt end;

public:
    using ParseState = ParseState;

    static bool is_number(char c)
    {
        return '0' <= c && c <= '9';
    }
    static bool is_white_space(char c)
    {
        return c == ' ' || c == '\t' || c == '\n';
    }
    static bool is_value_end(char c)
    {
        return c == ',';
    }

    ParseImpl(FwIt& begin, FwIt end)
        : begin(begin)
        , end(end)
    {
    }

    template <typename T> T parse(Type<T>);

    template <typename T> std::vector<T> parse(Type<std::vector<T>>);

    int parse(Type<int>);
    unsigned parse(Type<unsigned>);
    float parse(Type<float>);
    double parse(Type<double>);
    std::string parse(Type<std::string>);

private:
    template <typename TResult> TResult parse_float();
    void throw_unexpected_character(char chr);
    template <typename Fun> void skip_until(Fun&& predicate);
    template <typename T> void init();
    void assert_correct_value_end(char ending);
};

template <typename FwIt> template <typename T> T ParseImpl<FwIt>::parse(Type<T>)
{
    init<T>();
    auto result = T{};
    std::string key {};
    while (begin != end)
    {
        switch (state)
        {
        case ParseState::Default:
            skip_until([](auto c) { return !is_white_space(c); });
            if (*begin == '"')
            {
                state = ParseState::Key;
            }
            else if (*begin == '}')
            {
                ++begin;
                return result;
            }
            else
            {
                throw_unexpected_character(*begin);
            }
            break;
        case ParseState::Key:
            if (*begin == '"')
            {
                state = ParseState::Value;
                ++begin;
                skip_until([](auto c) { return !is_white_space(c); });
                if (*begin != ':')
                {
                    throw_unexpected_character(*begin);
                }
            }
            else
            {
                key += *begin;
            }
            break;
        case ParseState::Value:
            executeByPropertyName<T>(key.c_str(), [&](auto property) {
                using PropertyType = typename decltype(property)::Type;
                (PropertyType&)(result.*(property.member)) =
                    ParseImpl<FwIt>{begin, end}.parse(Type<PropertyType>{});
            });
            state = ParseState::Default;
            key.clear();
            skip_until([](auto c) { return !is_white_space(c); });
            assert_correct_value_end('}');
            continue;
        }
        ++begin;
    }
    throw ParseError("Unexpected end to the json input!");
}

template <typename FwIt>
template <typename T>
std::vector<T> ParseImpl<FwIt>::parse(Type<std::vector<T>>)
{
    skip_until([](auto c) { return !is_white_space(c); });
    if (*begin != '[')
    {
        throw_unexpected_character(*begin);
    }
    ++begin;
    auto result = std::vector<T>{};
    while (*begin != ']')
    {
        result.push_back(parse(Type<T>{}));
        skip_until([](auto c) { return !is_white_space(c); });
        assert_correct_value_end(']');
    }
    ++begin;
    return result;
}

template <typename FwIt> int ParseImpl<FwIt>::parse(Type<int>)
{
    skip_until([](auto c) { return !is_white_space(c); });
    auto stream = std::stringstream{};
    stream << *begin++;
    if (is_number(*begin))
    {
        stream << parse(Type<unsigned>{});
    }
    int result = 0;
    stream >> result;
    return result;
}

template <typename FwIt> unsigned ParseImpl<FwIt>::parse(Type<unsigned>)
{
    skip_until([](auto c) { return !is_white_space(c); });
    auto stream = std::stringstream{};
    while (begin != end && (is_number(*begin) && !is_value_end(*begin) && !is_white_space(*begin)))
    {
        stream << *begin;
        ++begin;
    }
    unsigned result = 0;
    stream >> result;
    return result;
}

template <typename FwIt> float ParseImpl<FwIt>::parse(Type<float>)
{
    return parse_float<float>();
}

template <typename FwIt> double ParseImpl<FwIt>::parse(Type<double>)
{
    return parse_float<double>();
}

template <typename FwIt> std::string ParseImpl<FwIt>::parse(Type<std::string>)
{
    skip_until([](auto c) { return !is_white_space(c); });
    if (*begin == '"')
    {
        ++begin;
    }
    else
    {
        throw_unexpected_character(*begin);
    }
    auto stream = std::stringstream{};
    stream << "\"";
    for (; begin != end && (*begin != '"' || stream.str().back() == '\\'); ++begin)
    {
        stream << *begin;
    }
    if (begin == end)
    {
        throw ParseError("Unexpected end to the json input!");
    }
    stream << "\"";
    auto result = std::string();
    stream >> std::quoted(result);
    ++begin;
    return result;
}

template <typename FwIt> template <typename TResult> TResult ParseImpl<FwIt>::parse_float()
{
    skip_until([](auto c) { return !is_white_space(c); });
    auto stream = std::stringstream{};
    stream << *begin++;
    while (begin != end && ((is_number(*begin) || *begin == '.') && !is_value_end(*begin) &&
                            !is_white_space(*begin)))
    {
        stream << *begin;
        ++begin;
    }
    TResult result;
    stream >> result;
    return result;
}

template <typename FwIt> void ParseImpl<FwIt>::throw_unexpected_character(char chr)
{
    using namespace std::string_literals;
    throw ParseError("Unexpected character: ["s + chr + "] in json input!");
}

template <typename FwIt> template <typename Fun> void ParseImpl<FwIt>::skip_until(Fun&& predicate)
{
    auto result = begin == end || predicate(*begin);
    while (!result)
    {
        ++begin;
        result = begin == end || predicate(*begin);
    }
    if (begin == end && !result)
    {
        throw ParseError("Unexpected end to the json input!");
    }
}

template <typename FwIt> template <typename T> void ParseImpl<FwIt>::init()
{
    static_assert(_private::IsJsonParseble<T>::value,
                  "Type must specify 'jsonProperties' static member "
                  "function to be used in this context!");
    state = ParseState::Default;
    skip_until([](auto c) { return !is_white_space(c); });
    if (*begin != '{')
    {
        throw_unexpected_character(*begin);
    }
    ++begin;
}
template <typename FwIt> void ParseImpl<FwIt>::assert_correct_value_end(char ending)
{
    if (*begin == ',')
    {
        ++begin;
    }
    else if (*begin != ending)
    {
        throw_unexpected_character(*begin);
    }
}
} // namespace mini_json::_private

