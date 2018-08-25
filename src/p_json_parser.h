#pragma once
#include "p_json_error.h"
#include "p_json_utility.h"
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace Mino::Json::Private {
template <typename FwIt>
class ParseImpl {
    enum class ParseState {
        Default,
        Key,
        Value
    };

    ParseState state = ParseState::Default;
    FwIt& begin;
    FwIt end;

public:
    using ParseState = ParseState;

    static bool isNumber(char c) { return '0' <= c && c <= '9'; }
    static bool isWhiteSpace(char c) { return c == ' ' || c == '\t' || c == '\n'; }
    static bool isValueEnd(char c) { return c == ','; }

    ParseImpl(FwIt& begin, FwIt end)
        : begin(begin)
        , end(end)
    {
    }

    template <typename T>
    T parse(Type<T>);

    template <typename T>
    std::vector<T> parse(Type<std::vector<T>>);

    int parse(Type<int>);
    unsigned parse(Type<unsigned>);
    float parse(Type<float>);
    double parse(Type<double>);
    std::string parse(Type<std::string>);

private:
    template <typename TResult>
    TResult parseFloat();
    void throwUnexpectedCharacter(char chr);
    template <typename Fun>
    void skipUntil(Fun&& predicate);
    template <typename T>
    void init();
    void assertCorrectValueEnd(char ending);
};

template <typename FwIt>
template <typename T>
T ParseImpl<FwIt>::parse(Type<T>)
{
    init<T>();
    auto result = T {};
    auto key = ""s;
    while (begin != end) {
        switch (state) {
        case ParseState::Default:
            skipUntil([](auto c) { return !isWhiteSpace(c); });
            if (*begin == '"') {
                state = ParseState::Key;
            } else if (*begin == '}') {
                ++begin;
                return result;
            } else {
                throwUnexpectedCharacter(*begin);
            }
            break;
        case ParseState::Key:
            if (*begin == '"') {
                state = ParseState::Value;
                ++begin;
                skipUntil([](auto c) { return !isWhiteSpace(c); });
                if (*begin != ':') {
                    throwUnexpectedCharacter(*begin);
                }
            } else {
                key += *begin;
            }
            break;
        case ParseState::Value:
            executeByPropertyName<T>(key.c_str(), [&](auto property) {
                using PropertyType = typename decltype(property)::Type;
                (PropertyType&)(result.*(property.member))
                    = ParseImpl<FwIt> { begin, end }.parse(Type<PropertyType> {});
            });
            state = ParseState::Default;
            key.clear();
            skipUntil([](auto c) { return !isWhiteSpace(c); });
            assertCorrectValueEnd('}');
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
    skipUntil([](auto c) { return !isWhiteSpace(c); });
    if (*begin != '[') {
        throwUnexpectedCharacter(*begin);
    }
    ++begin;
    auto result = std::vector<T> {};
    while (*begin != ']') {
        result.push_back(parse(Type<T> {}));
        skipUntil([](auto c) { return !isWhiteSpace(c); });
        assertCorrectValueEnd(']');
    }
    ++begin;
    return result;
}

template <typename FwIt>
int ParseImpl<FwIt>::parse(Type<int>)
{
    skipUntil([](auto c) { return !isWhiteSpace(c); });
    auto stream = std::stringstream {};
    stream << *begin++;
    if (isNumber(*begin)) {
        stream << parse(Type<unsigned> {});
    }
    int result = 0;
    stream >> result;
    return result;
}

template <typename FwIt>
unsigned ParseImpl<FwIt>::parse(Type<unsigned>)
{
    skipUntil([](auto c) { return !isWhiteSpace(c); });
    auto stream = std::stringstream {};
    while (begin != end && (isNumber(*begin) && !isValueEnd(*begin) && !isWhiteSpace(*begin))) {
        stream << *begin;
        ++begin;
    }
    unsigned result = 0;
    stream >> result;
    return result;
}

template <typename FwIt>
float ParseImpl<FwIt>::parse(Type<float>)
{
    return parseFloat<float>();
}

template <typename FwIt>
double ParseImpl<FwIt>::parse(Type<double>)
{
    return parseFloat<double>();
}

template <typename FwIt>
std::string ParseImpl<FwIt>::parse(Type<std::string>)
{
    skipUntil([](auto c) { return !isWhiteSpace(c); });
    if (*begin == '"') {
        ++begin;
    } else {
        throwUnexpectedCharacter(*begin);
    }
    auto stream = std::stringstream {};
    stream << "\"";
    for (; begin != end && (*begin != '"' || stream.str().back() == '\\'); ++begin) {
        stream << *begin;
    }
    if (begin == end) {
        throw ParseError("Unexpected end to the json input!");
    }
    stream << "\"";
    auto result = std::string();
    stream >> std::quoted(result);
    ++begin;
    return result;
}

template <typename FwIt>
template <typename TResult>
TResult ParseImpl<FwIt>::parseFloat()
{
    skipUntil([](auto c) { return !isWhiteSpace(c); });
    auto stream = std::stringstream {};
    stream << *begin++;
    while (begin != end
        && ((isNumber(*begin) || *begin == '.') && !isValueEnd(*begin) && !isWhiteSpace(*begin))) {
        stream << *begin;
        ++begin;
    }
    TResult result;
    stream >> result;
    return result;
}

template <typename FwIt>
void ParseImpl<FwIt>::throwUnexpectedCharacter(char chr)
{
    throw ParseError("Unexpected character: ["s + chr + "] in json input!");
}

template <typename FwIt>
template <typename Fun>
void ParseImpl<FwIt>::skipUntil(Fun&& predicate)
{
    auto result = begin == end || predicate(*begin);
    while (!result) {
        ++begin;
        result = begin == end || predicate(*begin);
    }
    if (begin == end && !result) {
        throw ParseError("Unexpected end to the json input!");
    }
}

template <typename FwIt>
template <typename T>
void ParseImpl<FwIt>::init()
{
    static_assert(Private::IsJsonParseble<T>::value,
        "Type must specify 'jsonProperties' static member "
        "function to be used in this context!");
    state = ParseState::Default;
    skipUntil([](auto c) { return !isWhiteSpace(c); });
    if (*begin != '{') {
        throwUnexpectedCharacter(*begin);
    }
    ++begin;
}
template <typename FwIt>
void ParseImpl<FwIt>::assertCorrectValueEnd(char ending)
{
    if (*begin == ',') {
        ++begin;
    } else if (*begin != ending) {
        throwUnexpectedCharacter(*begin);
    }
}
}

