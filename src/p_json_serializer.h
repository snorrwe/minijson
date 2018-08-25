#pragma once
#include "p_json_error.h"
#include "p_json_utility.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace MiniJson::Private
{
template <typename TStream> class SerializerImpl
{
    TStream& stream;

public:
    SerializerImpl(TStream& stream)
        : stream(stream)
    {
    }

    template <typename T> void serialize(T const& item)
    {
        stream << "{";

        constexpr auto nbProperties = std::tuple_size<decltype(T::jsonProperties())>::value;

        const char* separator = "";
        for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i) {
            constexpr auto property = std::get<i>(T::jsonProperties());
            stream << separator;
            serialize(std::string(property.name));
            stream << ":";
            serialize(item.*(property.member));
            separator = ",";
        });

        stream << "}";
    }

    template <typename T> void serialize(std::vector<T> const& items)
    {
        stream << "[";
        const char* separator = "";
        for (auto& item : items)
        {
            stream << separator;
            separator = ",";
            serialize(item);
        }
        stream << "]";
    }

    void serialize(std::string const& item)
    {
        stream << std::quoted(item);
    }
    void serialize(int item)
    {
        stream << item;
    }
    void serialize(unsigned item)
    {
        stream << item;
    }
    void serialize(float item)
    {
        stream << item;
    }
    void serialize(double item)
    {
        stream << item;
    }
};
} // namespace MiniJson::Private

